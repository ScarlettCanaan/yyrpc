#include "tcp_sync_client_transport.h"
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <unistd.h>
#endif

_START_ORPC_NAMESPACE_

TcpSyncClientTransport::TcpSyncClientTransport(IPacketCallback* callback, int32_t flags)
: m_packetCallback(callback), m_connMonitor(nullptr), m_fd(-1)
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

TcpSyncClientTransport::~TcpSyncClientTransport()
{
	Close(0);
}

int TcpSyncClientTransport::Send(uint16_t msg_type, const char* data, size_t len)
{
  if (len <= Packet::MAX_BODY_LENGTH)
    return _Send(msg_type, data, len);
  else if (len <= LargePacket::MAX_BODY_LENGTH)
    return _SendLarge(msg_type, data, len);

  ORPC_LOG(ERROR) << "send size must less than " << LargePacket::MAX_BODY_LENGTH << " msg_type:" << msg_type;
  return -1;
}

int TcpSyncClientTransport::Send(const char* data, size_t len)
{
  return DoSend(data, len);
}

int TcpSyncClientTransport::_Send(uint16_t msg_type, const char* data, size_t len)
{
  Packet* packet = (Packet*)g_packetPool.allocate();
  packet->setMsgType(msg_type);
  uint16_t totalLen = len + Packet::PACKET_HEAD_LENGTH;
  packet->setTotalLength(totalLen);
  packet->setBodyData(data, len);

  return _Send((Packet*)packet);
}

int TcpSyncClientTransport::_Send(Packet* packet)
{
  char* buf = (char*)packet;
  int total = packet->getTotalLength();
  return DoSend(buf, total);
}
#ifdef WIN32
#define WHILE_RETYR(ret) while (ret == SOCKET_ERROR && GetLastError() == WSAEWOULDBLOCK);
#else
#define WHILE_RETYR(ret) while (ret < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK));
#endif

int TcpSyncClientTransport::Send(uint16_t msg_type, BufferStream& buff)
{
  size_t len = buff.size();
  char* data = buff.release();

  int16_t* plen = (int16_t*)data;
  int16_t* ptype = (int16_t*)(data + 2);
  *plen = htons(len + Packet::PACKET_HEAD_LENGTH);
  *ptype = htons(msg_type);
  return DoSend(data, len + Packet::PACKET_HEAD_LENGTH);
}

int TcpSyncClientTransport::DoSend(const char* data, size_t len)
{
  size_t curr_read = 0;
  int written = 0;
  while (curr_read < len)
  {
    do { 
      written = ::send(m_fd, data + curr_read, len - curr_read, 0);
    } WHILE_RETYR(written)

    if (written < 0)
      return -1;
    curr_read += written;
  }

  int readcn = 0;
  while (m_connMonitor 
    && m_connMonitor->NeedWaitResult())
  {
    size_t bufSize;
    char* recvBuf = m_decoder->GetWriteBuffer(bufSize);
    do { 
      readcn = ::recv(m_fd, recvBuf, bufSize, 0);
    } WHILE_RETYR(readcn)

    if (readcn <= 0)
      return -1;
    m_decoder->Process(recvBuf, readcn);
  }

  return 0;
}

int TcpSyncClientTransport::_SendLarge(uint16_t msg_type, const char* data, size_t len)
{
  LargePacket* packet = (LargePacket*)malloc(sizeof(LargePacket));
  packet->setMsgType(msg_type);
  uint16_t totalLen = len + LargePacket::PACKET_HEAD_LENGTH;
  packet->setTotalLength(totalLen);
  packet->setBodyData(data, len);

  return _Send((Packet*)packet);
}

int TcpSyncClientTransport::Close(int reason)
{
  if (m_connectStatu != CS_CONNECTED)
    return 0;

  OnRequireClose(reason);

#ifdef WIN32
  if (m_fd != INVALID_SOCKET)
  {
    ::closesocket(m_fd);
    m_fd = INVALID_SOCKET;
  }
#else
  if (m_fd != -1)
  {
    ::close(m_fd);
    m_fd = -1;
  }
#endif  //! #ifdef WIN32

  m_connectStatu = CS_CLOSED;
  OnAfterClose();
  return 0;
}

int TcpSyncClientTransport::OnConnected(bool bSucc)
{
  if (m_connMonitor)
    return m_connMonitor->OnConnected(bSucc);
  return 0;
}

int TcpSyncClientTransport::OnRequireClose(int reason)
{
  if (m_connMonitor)
    return m_connMonitor->OnRequireClose(reason);
  return 0;
}

int TcpSyncClientTransport::OnAfterClose()
{
  if (m_connMonitor)
    return m_connMonitor->OnAfterClose();
  return 0;
}

int TcpSyncClientTransport::Connect(const std::string& ip, int port, uv_loop_t* loop)
{
  m_ip = ip;
  m_port = port;

  int r = DoConnect();
  m_connectStatu = (r < 0) ? CS_FAIL : CS_CONNECTED;
  OnConnected(r == 0);
  
  return r;
}

int TcpSyncClientTransport::DoConnect()
{
  m_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (m_fd == -1)
  {
    ORPC_LOG(ERROR) << "socket create fail";
    return -1;
  }

#ifdef WIN32
  int timeout = 3000; //3s
  if (setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) == -1)
  {
    ORPC_LOG(ERROR) << "setsockopt set send timeout fail";
    return -1;
  }
#else
  // set connect timeout (connect timeout use SO_SNDTIMO value)
  struct timeval connectTimeout = { 2, 0 }; // 2 seconds
  if (setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, &connectTimeout, sizeof(connectTimeout)) == -1)
  {
    ORPC_LOG(ERROR) << "setsockopt set connect timeout fail";
    return -1;
  }
#endif  //! #ifdef WIN32

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_addr.s_addr = inet_addr(m_ip.c_str());
  sin.sin_family = AF_INET;
  sin.sin_port = htons(m_port);

  int res = ::connect(m_fd, (struct sockaddr *)&sin, sizeof(sin));
  if (res == -1)
  {
    Close(0);
    return -1;
  }

#ifdef WIN32
  if (setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) == -1)
  {
    ORPC_LOG(ERROR) << "setsockopt set send timeout fail";
    return -1;
  }

  if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1)
  {
    ORPC_LOG(ERROR) << "setsockopt set recv timeout fail";
    return -1;
  }
#else
  //set send and recv timeout
  struct timeval dataTimeout = { 3, 0 };
  if (setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, &dataTimeout, sizeof(dataTimeout)) == -1)
  {
    ORPC_LOG(ERROR) << "setsockopt set send timeout fail";
    return -1;
  }

  if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &dataTimeout, sizeof(dataTimeout)) == -1)
  {
    ORPC_LOG(ERROR) << "setsockopt set recv timeout fail";
    return -1;
  }
#endif  //! #ifdef WIN32

  int option = 1;
  if (setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&option, sizeof(option)))
  {
    ORPC_LOG(ERROR) << "setsockopt set TCP_NODELAY fail";
    return -1;
  }

  int recvbuf = 1024 * 1024;
  if (setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (char *)&recvbuf, sizeof(recvbuf)))
  {
    ORPC_LOG(ERROR) << "setsockopt set SO_RCVBUF fail";
    return -1;
  }

  int sendbuf = 1024 * 1024;
  if (setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, (char *)&sendbuf, sizeof(sendbuf)))
  {
    ORPC_LOG(ERROR) << "setsockopt set SO_SNDBUF fail";
    return -1;
  }

  return 0;
}

_END_ORPC_NAMESPACE_
