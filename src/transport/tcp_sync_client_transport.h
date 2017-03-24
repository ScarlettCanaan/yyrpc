#ifndef _TCP_SYNC_CLIENT_TRANSPORT_H_
#define _TCP_SYNC_CLIENT_TRANSPORT_H_

#include "../build_config.h"
#include <string>
#include "transport.h"
#include "../packet/tcp_packet_callback.h"

_START_ORPC_NAMESPACE_

class TcpSyncClientTransport : public IClientTransport
{
public:
  TcpSyncClientTransport(IPacketCallback* callback, int32_t flags = MP_BINARY);
  virtual ~TcpSyncClientTransport();
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
private:
  int _Send(uint16_t msg_type, const char* data, size_t len);
  int _SendLarge(uint16_t msg_type, const char* data, size_t len);
  int _Send(Packet* packet);
  int DoConnect();
  int DoSend(const char* data, size_t len);
private:
  int                   m_fd;
  std::string           m_ip;
  unsigned short        m_port;
  ConnectStatus         m_connectStatu;
  PacketDecoder*        m_decoder;
  IPacketCallback*      m_packetCallback;
  IConnectionMonitor *  m_connMonitor;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef _TCP_SYNC_CLIENT_TRANSPORT_H_


