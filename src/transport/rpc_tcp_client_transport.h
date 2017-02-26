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
  RpcTcpClientTransport();
  ~RpcTcpClientTransport();
public:
  virtual int OnRecvPacket(const Packet* rawPacket) override;
  virtual int OnRequireClose(int reason) override { return 0; }
  virtual int OnAfterClose() override { return 0; }
private:
  int ProcessResult(const Packet* rawPacket);
  int ProcessEvent(const Packet* rawPacket);

  virtual int OnProcessResult(const Packet* rawPacket) = 0;
  virtual int OnProcessEvent(const Packet* rawPacket) = 0;
private:
  typedef int (RpcTcpClientTransport::*ProcFun)(const Packet* rawPacket);
  ProcFun m_procFunc[YYRPC_PROTOCAL_MAX];
};

#endif  //! #ifndef RPC_TCP_CLIENT_TRANPORT_H_
