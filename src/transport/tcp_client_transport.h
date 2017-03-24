#ifndef _C2G_CONNECTION_H_
#define _C2G_CONNECTION_H_

#include "../build_config.h"
#include "uv.h"
#include "../packet/tcp_packet_decoder.h"
#include "../packet/tcp_packet_callback.h"
#include "transport.h"
#include "../buff_stream.h"

_START_ORPC_NAMESPACE_

class TcpClientTransport : public IClientTransport
{
public:
  TcpClientTransport(IPacketCallback* callback, int32_t flags = MP_BINARY);
  ~TcpClientTransport();
public:
  virtual int Send(uint16_t msg_type, const char* data, size_t len) override;
  virtual int Send(const char* data, size_t len) override;
  virtual int Send(uint16_t msg_type, BufferStream& buff) override;
  virtual int Close(int reason) override;
  virtual ConnectStatus GetConnectStatus() const override { return m_connectStatu; }
  virtual int Connect(const std::string& ip, int port, uv_loop_t* loop = 0) override;
  virtual void SetConnectMonitor(IConnectionMonitor* monitor) override { m_connMonitor = monitor; }
public:
  virtual int OnConnected(bool bSucc);
  virtual int OnRequireClose(int reason);
  virtual int OnAfterClose();
public:
  int OnRead(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf);

  PacketDecoder* decoder() { return m_decoder; }
  uv_tcp_t* storage() { return &m_tcp; }
public:
  int _OnConnected(uv_connect_t* req, int status);
private:
  int _Send(uint16_t msg_type, const char* data, size_t len);
  int _SendLarge(uint16_t msg_type, const char* data, size_t len);
  int _Send(Packet* packet);
protected:
  sockaddr_in           m_listenAddr;
  uv_tcp_t              m_tcp;
  uv_connect_t          m_connectReq;
  ConnectStatus         m_connectStatu;
  PacketDecoder*        m_decoder;
  IPacketCallback*      m_packetCallback;
  IConnectionMonitor*   m_connMonitor;
};

_END_ORPC_NAMESPACE_

#endif // !_C2G_CONNECTION_H_
