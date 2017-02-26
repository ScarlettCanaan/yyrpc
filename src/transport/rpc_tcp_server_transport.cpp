#include "rpc_tcp_server_transport.h"
#include "glog/logging.h"
#include "util/util.h"
#include <assert.h>
#include "../proto/common_api.h"
#include "msgpack.hpp"
#include "listenpoint/rpc_client_accept.h"

RpcTcpServerTransport::RpcTcpServerTransport(RpcClientAccept* accptor)
  :m_accptor(accptor)
{
  memset(m_procFunc, 0, sizeof(ProcFun) * YYRPC_PROTOCAL_MAX);
  m_procFunc[YYRPC_PROTOCAL_CALL] = &RpcTcpServerTransport::ProcessCall;
}

RpcTcpServerTransport::~RpcTcpServerTransport()
{

}

int RpcTcpServerTransport::OnRecvPacket(const Packet* rawPacket)
{
  uint16_t msgType = rawPacket->getMsgType();
  if (m_procFunc[msgType])
    return (this->*m_procFunc[msgType])(rawPacket);

  LOG(ERROR) << "unknown msg_type: " << msgType;
  return CR_INVALID_PACKET;
}

int RpcTcpServerTransport::ProcessCall(const Packet* rawPacket)
{
  std::string s(rawPacket->getBodyData(), rawPacket->getBodyLength());

  std::shared_ptr<msgpack::unpacker> unp = std::make_shared<msgpack::unpacker>();
  unp->reserve_buffer(s.size());
  std::stringstream input(s);
  std::size_t actual_read_size = input.readsome(unp->buffer(), s.size());
  if (actual_read_size != s.size())
    return -1;

  unp->buffer_consumed(actual_read_size);

  int32_t session_id = 0;
  std::string method_name;
  if (!unserialization_header(*unp, session_id, method_name))
    return -1;

  if (!CalleeManager::GetInstance().OnCall(shared_from_this(), session_id, method_name, unp))
    return -1;

  return 0;
}

int RpcTcpServerTransport::OnRequireClose(int reason)
{
  if (m_connectStatu == CS_CLOSED)
    return -1;

  if (reason != 0)
  {
    SocketDisconnectMessage close;
    close.message_id = reason;
    std::stringstream s;
    if (serialization_result_header(s, 0, "", 0))
      SendResult(s);
  }

  return 0;
}

int RpcTcpServerTransport::OnAfterClose()
{
  m_accptor->UnRegisterTransport(shared_from_this());
  return 0;
}

bool RpcTcpServerTransport::SendResult(std::stringstream& s)
{
  return m_accptor->QueueSend(shared_from_this(), s);
}
