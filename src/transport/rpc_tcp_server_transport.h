#ifndef RPC_TCP_SERVER_TRANSPORT_H_
#define RPC_TCP_SERVER_TRANSPORT_H_

#include "uv.h"
#include "transport/tcp_server_transport.h"
#include "protocal_def.h"

class RpcTcpServerTransport : public TcpServerTransport
{
public:
  RpcTcpServerTransport();
  ~RpcTcpServerTransport();
public:
  virtual int OnRecvPacket(const Packet* rawPacket) override;
  virtual int OnRequireClose(int reason) override;
  virtual int OnAfterClose() override;
private:
  int ProcessCall(const Packet* rawPacket);
private:
  typedef int (RpcTcpServerTransport::*ProcFun)(const Packet* rawPacket);
  ProcFun m_procFunc[YYRPC_PROTOCAL_MAX];
};

#endif  //! #ifndef RPC_TCP_SERVER_TRANSPORT_H_
