#include "rpc_tcp_client_transport.h"
#include "util/util.h"

RpcTcpClientTransport::RpcTcpClientTransport()
{
  memset(m_procFunc, 0, sizeof(ProcFun) * YYRPC_PROTOCAL_MAX);
  m_procFunc[YYRPC_PROTOCAL_RESULT] = &RpcTcpClientTransport::ProcessResult;
  m_procFunc[YYRPC_PROTOCAL_EVENT] = &RpcTcpClientTransport::ProcessEvent;
}

RpcTcpClientTransport::~RpcTcpClientTransport()
{

}

int RpcTcpClientTransport::OnRecvPacket(const Packet* rawPacket)
{
  uint16_t msgType = rawPacket->getMsgType();
  if (m_procFunc[msgType])
    return (this->*m_procFunc[msgType])(rawPacket);

  LOG(ERROR) << "unknown msg_type: " << msgType;
  return -1;
}

int RpcTcpClientTransport::ProcessResult(const Packet* rawPacket)
{
  return OnProcessResult(rawPacket);
}

int RpcTcpClientTransport::ProcessEvent(const Packet* rawPacket)
{
  return OnProcessEvent(rawPacket);
}
