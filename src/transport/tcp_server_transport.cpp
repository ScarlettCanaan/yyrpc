#include "tcp_server_transport.h"
#include "../util/base_util.h"
#include "../packet/packet.h"
#include <assert.h>

_START_ORPC_NAMESPACE_

TcpServerTransport::TcpServerTransport(int32_t flags)
{
  if (IS_HTTP(flags))
  {
#ifdef ORPC_SUPPROT_HTTP
    m_decoder = new HttpPacketDecoder(this, HPT_REUQEST);
#else
    throw std::runtime_error("not support http.");
#endif  //! #ifdef ORPC_SUPPROT_HTTP
  } 
  else if (IS_BINARY(flags))
  {
    m_decoder = new BinaryPacketDecoder(this);
  }
  else
  {
    throw std::runtime_error("unknown method.");
  }
  m_connectStatu = CS_INIT;
  m_connectedTime = -1;
  m_peerPort = -1;
}

TcpServerTransport::~TcpServerTransport()
{
  delete m_decoder;
}

static void write_cb(uv_write_t* req, int status)
{
  if (status != 0)
    ORPC_LOG(ERROR) << "write cb error " << uv_strerror(status);

  TcpWriter* data = (TcpWriter*)req;
  if (status != 0)
  {
    TcpServerTransport* conn = (TcpServerTransport*)data->conn;
    conn->Close(CR_PEER_CLOSED);
  }
  data->Release();
}

int TcpServerTransport::_Send(uint16_t msg_type, const char* data, size_t len)
{
  Packet* packet = (Packet*)g_packetPool.allocate();
  packet->setMsgType(msg_type);
  uint16_t totalLen = len + Packet::PACKET_HEAD_LENGTH;
  packet->setTotalLength(totalLen);
  packet->setBodyData(data, len);

  return _Send(packet);
}

int TcpServerTransport::_SendLarge(uint16_t msg_type, const char* data, size_t len)
{
  LargePacket* packet = (LargePacket*)malloc(sizeof(LargePacket));
  packet->setMsgType(msg_type);
  uint16_t totalLen = len + LargePacket::PACKET_HEAD_LENGTH;
  packet->setTotalLength(totalLen);
  packet->setBodyData(data, len);

  return _Send((Packet*)packet);
}

int TcpServerTransport::Send(const char* data, size_t len)
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

int TcpServerTransport::Send(uint16_t msg_type, const char* data, size_t len)
{
  if (len <= Packet::MAX_BODY_LENGTH)
    return _Send(msg_type, data, len);
  else if (len <= LargePacket::MAX_BODY_LENGTH)
    return _SendLarge(msg_type, data, len);
  
  ORPC_LOG(ERROR) << "send size must less than " << LargePacket::MAX_BODY_LENGTH;
  return -1;
}

int TcpServerTransport::_Send(Packet* packet)
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

int TcpServerTransport::Send(uint16_t msg_type, BufferStream& buff)
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

void TcpServerTransport::SetConnected()
{
  m_connectStatu = CS_CONNECTED;
  GetTimeSecond(&m_connectedTime);
  struct sockaddr peer_name;
  int namelen = sizeof(sockaddr);
  uv_tcp_getpeername(&m_tcp, &peer_name, &namelen);
  
  char check_ip[64] = { 0 };
  uv_ip4_name((struct sockaddr_in*)&peer_name, (char*)check_ip, sizeof check_ip);
  m_peerIp = check_ip;
  m_peerPort = ntohs(((struct sockaddr_in*)&peer_name)->sin_port);
}

int TcpServerTransport::OnRead(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
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
  TcpServerTransport* peer = (TcpServerTransport*)handle->data;
  peer->OnAfterClose();
}

int TcpServerTransport::Close(int reason)
{
  if (m_connectStatu == CS_CLOSED)
    return -1;

  int r = OnRequireClose(reason);
  if (r < 0)
    return -1;

  ORPC_LOG(ERROR) << "close connect, reason: " << reason;

  m_connectStatu = CS_CLOSED;
  m_decoder->Reset();
  m_connectedTime = -1;
  uv_close((uv_handle_t*)&m_tcp, close_cb);

  return 0;
}

_END_ORPC_NAMESPACE_
