#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

#include "../build_config.h"
#include "uv.h"
#include "../packet/tcp_packet_decoder.h"
#include "../packet/tcp_packet_callback.h"
#include "transport.h"
#include "../buff_stream.h"

_START_ORPC_NAMESPACE_

class AcceptWorker;

class TcpServerTransport : public ITransport, public IPacketCallback
{
public:
  TcpServerTransport(int32_t flags = MP_BINARY);
  ~TcpServerTransport();
public:
  virtual int Send(uint16_t msg_type, const char* data, size_t len) override;
  virtual int Send(const char* data, size_t len) override;
  virtual int Send(uint16_t msg_type, BufferStream& buff) override;
  virtual int Close(int reason) override;
  virtual ConnectStatus GetConnectStatus() const override { return m_connectStatu; }
  const std::string& GetPeerIp() const { return m_peerIp; }
  int32_t GetPeerPort() { return m_peerPort; }
public:
  virtual int OnRequireClose(int reason) { return 0; }
  virtual int OnAfterClose() { return 0; }
public:
  int OnRead(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf);

  void SetConnected();

  PacketDecoder* decoder() { return m_decoder; }
  uv_tcp_t* storage() { return &m_tcp;  }
private:
  int _Send(uint16_t msg_type, const char* data, size_t len);
  int _SendLarge(uint16_t msg_type, const char* data, size_t len);
  int _Send(Packet* packet);
protected:
  std::string   m_peerIp;
  int32_t  m_peerPort;
  uv_tcp_t m_tcp;
  PacketDecoder* m_decoder;
  ConnectStatus m_connectStatu;
  int64_t  m_connectedTime;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef _TCP_CONNECTION_H_
