#include "tcp_client_transport.h" 
#include "../util/base_util.h"
#include "../packet/packet.h"

_START_ORPC_NAMESPACE_

TcpClientTransport::TcpClientTransport(IPacketCallback* callback, int32_t flags)
: m_packetCallback(callback), m_connMonitor(nullptr)
{
  if (IS_HTTP(flags))
  {
#ifdef ORPC_SUPPROT_HTTP
    m_decoder = new HttpPacketDecoder(m_packetCallback, HPT_RESPONSE);
#else
    throw std::runtime_error("not support http.");
#endif  //! #ifdef ORPC_SUPPROT_HTTP
  }
  else if (IS_BINARY(flags))
  {
    m_decoder = new BinaryPacketDecoder(m_packetCallback);
  }
  else
  {
    throw std::runtime_error("unknown method.");
  }
  m_connectStatu = CS_INIT;
}

TcpClientTransport::~TcpClientTransport()
{
  delete m_decoder;
}

static void cl_connect_cb(uv_connect_t* req, int status)
{
  TcpClientTransport* conn = (TcpClientTransport*)req->data;
  conn->_OnConnected(req, status);
}

static void cl_alloc_cb(uv_handle_t* handle,
  size_t suggested_size,
  uv_buf_t* buf)
{
  TcpClientTransport* peer = (TcpClientTransport*)handle->data;
  size_t len = 0;
  buf->base = peer->decoder()->GetWriteBuffer(len);
  buf->len = len;
}

static void cl_read_cb(uv_stream_t* handle,
  ssize_t nread,
  const uv_buf_t* buf)
{
  TcpClientTransport* peer = (TcpClientTransport*)handle->data;
  peer->OnRead(handle, nread, buf);
}

static void write_cb(uv_write_t* req, int status)
{
  if (status != 0)
    ORPC_LOG(ERROR) << "write cb error " << uv_strerror(status);

  TcpWriter* data = (TcpWriter*)req;
  if (status != 0)
  {
    TcpClientTransport* conn = (TcpClientTransport*)data->conn;
    conn->Close(CR_PEER_CLOSED);
  }
  data->Release();
}

int TcpClientTransport::Connect(const std::string& ip, int port, uv_loop_t* loop)
{
  int r;

  r = uv_ip4_addr(ip.c_str(), port, &m_listenAddr);
  UV_CHECK_RET_1(uv_ip4_addr, r);

  m_tcp.data = this;
  r = uv_tcp_init(loop ? loop : uv_default_loop(), (uv_tcp_t*)&m_tcp);
  UV_CHECK_RET_1(uv_tcp_init, r);

  uv_tcp_nodelay(&m_tcp, true);

  m_connectReq.data = this;
  m_connectStatu = CS_CONNECTING;

  r = uv_tcp_connect(&m_connectReq,
    (uv_tcp_t*)&m_tcp,
    (const struct sockaddr*) &m_listenAddr,
    cl_connect_cb);
  UV_CHECK_RET_1(uv_tcp_connect, r);

  return 0;
}

int TcpClientTransport::_OnConnected(uv_connect_t* req, int status)
{
  int r;

  if (status != 0)
  {
    m_connectStatu = CS_FAIL;
    ORPC_LOG(ERROR) << "on connect error " << uv_err_name(status);
    OnConnected(false);
    return -1; 
  }
  
  r = uv_read_start((uv_stream_t*)&m_tcp, cl_alloc_cb, cl_read_cb);
  UV_CHECK_RET_1(uv_read_start, r);

  m_connectStatu = CS_CONNECTED;

  m_decoder->Reset();
  OnConnected(true);

  return 0;
}

int TcpClientTransport::OnConnected(bool bSucc)
{
  if (m_connMonitor)
    return m_connMonitor->OnConnected(bSucc);
  return 0;
}

int TcpClientTransport::OnRequireClose(int reason) 
{ 
  if (m_connMonitor)
    return m_connMonitor->OnRequireClose(reason);
  return 0;
}

int TcpClientTransport::OnAfterClose()
{
  if (m_connMonitor)
    return m_connMonitor->OnAfterClose();
  return 0;
}

int TcpClientTransport::Send(const char* data, size_t len)
{
  int r;

  char* cdata = (char*)malloc(len);
  memcpy(cdata, data, len);
  TcpWriter* writeReq = new TcpWriter(this, 0, cdata);

  uv_buf_t buf = uv_buf_init((char*)cdata, len);
  r = uv_write((uv_write_t*)writeReq,
    (uv_stream_t*)&m_tcp,
    &buf,
    1,
    write_cb);

  if (r < 0)
  {
    writeReq->Release();
    ORPC_LOG(ERROR) << "uv_write error: " << uv_err_name(r);
    return r;
  }

  return 0;
}

int TcpClientTransport::_Send(uint16_t msg_type, const char* data, size_t len)
{
  Packet* packet = (Packet*)g_packetPool.allocate();
  packet->setMsgType(msg_type);
  uint16_t totalLen = len + Packet::PACKET_HEAD_LENGTH;
  packet->setTotalLength(totalLen);
  packet->setBodyData(data, len);

  return _Send((Packet*)packet);
}

int TcpClientTransport::_Send(Packet* packet)
{
  int r;

  TcpWriter* writeReq = new TcpWriter(this, packet, nullptr);

  uv_buf_t buf = uv_buf_init((char*)packet, packet->getTotalLength());
  r = uv_write((uv_write_t*)writeReq,
    (uv_stream_t*)&m_tcp,
    &buf,
    1,
    write_cb);

  if (r < 0)
  {
    writeReq->Release();
    ORPC_LOG(ERROR) << "uv_write error: " << uv_err_name(r);
    return r;
  }

  return 0;
}

int TcpClientTransport::_SendLarge(uint16_t msg_type, const char* data, size_t len)
{
  LargePacket* packet = (LargePacket*)malloc(sizeof(LargePacket));
  packet->setMsgType(msg_type);
  uint16_t totalLen = len + LargePacket::PACKET_HEAD_LENGTH;
  packet->setTotalLength(totalLen);
  packet->setBodyData(data, len);

  return _Send((Packet*)packet);
}

int TcpClientTransport::Send(uint16_t msg_type, const char* data, size_t len)
{
  if (len <= Packet::MAX_BODY_LENGTH)
    return _Send(msg_type, data, len);
  else if (len <= LargePacket::MAX_BODY_LENGTH)
    return _SendLarge(msg_type, data, len);

  ORPC_LOG(ERROR) << "send size must less than " << LargePacket::MAX_BODY_LENGTH << " msg_type:" << msg_type;
  return -1;
}

int TcpClientTransport::Send(uint16_t msg_type, BufferStream& buff)
{
  int r;

  size_t len = buff.size();
  char* data = buff.release();

  TcpWriter* writeReq = new TcpWriter(this, 0, data);

  int16_t* plen = (int16_t*)data;
  int16_t* ptype = (int16_t*)(data + 2);
  *plen = htons(len + Packet::PACKET_HEAD_LENGTH);
  *ptype = htons(msg_type);

  uv_buf_t buf = uv_buf_init(data, len + Packet::PACKET_HEAD_LENGTH);
  r = uv_write((uv_write_t*)writeReq,
    (uv_stream_t*)&m_tcp,
    &buf,
    1,
    write_cb);

  if (r < 0)
  {
    writeReq->Release();
    ORPC_LOG(ERROR) << "uv_write error: " << uv_err_name(r);
    return r;
  }

  return 0;
}

int TcpClientTransport::OnRead(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
  int r;
  if (nread < 0)
  {
    ORPC_LOG(ERROR) << "EOF read on tcp connection: " << uv_strerror(nread);
    Close(CR_PEER_CLOSED);
    return -1;
  }
  else if (nread == 0)
  {
    return 0;
  }

  r = m_decoder->Process(buf->base, nread);
  if (r != 0)
  {
    Close(r);
    return -1;
  }

  return 0;
}

static void close_cb(uv_handle_t* handle)
{
  TcpClientTransport* peer = (TcpClientTransport*)handle->data;
  peer->OnAfterClose();
}

int TcpClientTransport::Close(int reason)
{
  if (m_connectStatu == CS_CLOSED)
    return -1;

  int r = OnRequireClose(reason);
  if (r < 0)
    return -1;

  ORPC_LOG(ERROR) << "close connect, reason: " << reason;

  m_decoder->Reset();
  m_connectStatu = CS_CLOSED;
  uv_close((uv_handle_t*)&m_tcp, close_cb);
  return 0;
}

_END_ORPC_NAMESPACE_
