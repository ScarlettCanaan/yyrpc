#include "rpc_tcp_client_transport.h"
#include "util/util.h"

RpcTcpClientTransport::RpcTcpClientTransport(MethodProtocol mProtocal)
  : TcpClientTransport(mProtocal)
{
  memset(m_procFunc, 0, sizeof(ProcFun) * YYRPC_PROTOCAL_MAX);
  m_procFunc[YYRPC_PROTOCAL_RESULT] = &RpcTcpClientTransport::ProcessResult;
}

RpcTcpClientTransport::~RpcTcpClientTransport()
{

}

int RpcTcpClientTransport::OnRecvPacket(uint32_t msgType, const char* data, int32_t len)
{
  if (msgType < _countof(m_procFunc) && m_procFunc[msgType])
    return (this->*m_procFunc[msgType])(data, len);

  LOG(ERROR) << "unknown msg_type: " << msgType;
  return -1;
}

int RpcTcpClientTransport::OnRecvPacket(const std::string& msgType, const char* data, int32_t len)
{
  return ProcessResult(data, len);
}

int RpcTcpClientTransport::ProcessResult(const char* data, int32_t len)
{
  return OnProcessResult(data, len);
}

