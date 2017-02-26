#ifndef RPC_TCP_SERVER_TRANSPORT_H_
#define RPC_TCP_SERVER_TRANSPORT_H_

#include "uv.h"
#include "transport/tcp_server_transport.h"
#include "protocal_def.h"
#include <memory>

class RpcClientAccept;

class RpcTcpServerTransport : public TcpServerTransport, public std::enable_shared_from_this<RpcTcpServerTransport>
{
public:
  RpcTcpServerTransport(RpcClientAccept* accptor);
  ~RpcTcpServerTransport();
public:
  virtual int OnRecvPacket(const Packet* rawPacket) override;
  virtual int OnRequireClose(int reason) override;
  virtual int OnAfterClose() override;

  bool SendResult(std::stringstream& s);
private:
  int ProcessCall(const Packet* rawPacket);
private:
  RpcClientAccept* m_accptor;
  typedef int (RpcTcpServerTransport::*ProcFun)(const Packet* rawPacket);
  ProcFun m_procFunc[YYRPC_PROTOCAL_MAX];
};

#endif  //! #ifndef RPC_TCP_SERVER_TRANSPORT_H_
