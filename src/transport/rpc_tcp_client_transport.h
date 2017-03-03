#ifndef RPC_TCP_CLIENT_TRANPORT_H_
#define RPC_TCP_CLIENT_TRANPORT_H_

#include "uv.h"
#include "transport/tcp_client_transport.h"
#include "../proto/proto_queueserver_enum_def.h"
#include "protocal_def.h"
#include <memory>

class RpcTcpClientTransport : public TcpClientTransport
{
public:
  RpcTcpClientTransport(MethodProtocol mProtocal);
  ~RpcTcpClientTransport();
public:
  virtual int OnRecvPacket(uint32_t msgType, const char* data, int32_t len) override;
  virtual int OnRecvPacket(const std::string& msgType, const char* data, int32_t len) override;
  virtual int OnRequireClose(int reason) override { return 0; }
  virtual int OnAfterClose() override { return 0; }
private:
  int ProcessResult(const char* data, int32_t len);

  virtual int OnProcessResult(const char* data, int32_t len) = 0;
private:
  typedef int (RpcTcpClientTransport::*ProcFun)(const char* data, int32_t len);
  ProcFun m_procFunc[YYRPC_PROTOCAL_MAX];
};

#endif  //! #ifndef RPC_TCP_CLIENT_TRANPORT_H_
