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
  RpcTcpServerTransport(RpcClientAccept* accptor, MethodProtocol mProtocal);
  ~RpcTcpServerTransport();
public:
  virtual int OnRecvPacket(uint32_t msgType, const char* data, int32_t len) override;
  virtual int OnRecvPacket(const std::string& msgType, const char* data, int32_t len) override;
  virtual int OnRequireClose(int reason) override;
  virtual int OnAfterClose() override;

  bool SendResult(std::stringstream& s);

  bool SendError(int64_t session_id,
    const std::string& method_name_,
    int32_t errNo);
private:
  int ProcessCall(const char* data, int32_t len);
private:
  RpcClientAccept* m_accptor;
  typedef int (RpcTcpServerTransport::*ProcFun)(const char* data, int32_t len);
  ProcFun m_procFunc[YYRPC_PROTOCAL_MAX];
};

#endif  //! #ifndef RPC_TCP_SERVER_TRANSPORT_H_
