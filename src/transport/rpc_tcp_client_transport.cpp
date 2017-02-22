#include "rpc_tcp_client_transport.h"
#include "util/util.h"

RpcTcpClientTransport::RpcTcpClientTransport()
{
  /*memset(m_procFunc, 0, sizeof(ProcFun) * C2Q_MAX);
  m_procFunc[Q2C_UPDATE] = &RpcTcpClientTransport::ProcessQ2cUpdate;
  m_procFunc[Q2C_PASS] = &RpcTcpClientTransport::ProcessQ2cPass;
  m_procFunc[Q2C_CLOSE_MSG] = &RpcTcpClientTransport::ProcessQ2cClose;*/
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

int RpcTcpClientTransport::OnConnected(bool bSucc)
{
  /*char buffer[128] = { 0 };

  c2q_hello hello;
  hello.set_id(m_uid);
  hello.set_session("8234567890");
  hello.set_gatewayid(1);
  if (!hello.SerializeToArray(buffer, 128))
  return -1;

  int r;
  r = Send(C2Q_HELLO, (const char*)buffer, hello.ByteSize());
  UV_CHECK_RET_1(send, r);*/
  return 0;
}
