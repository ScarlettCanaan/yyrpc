#include "rpc_client_accept.h"
#include <algorithm>
#include "../transport/rpc_tcp_server_transport.h"
#include "util/util.h"

ResultPacket::ResultPacket(const std::weak_ptr<RpcTcpServerTransport>& conn_, std::stringstream& s)
  :conn(conn_), data(s.str())
{
 
}

RpcClientAccept::RpcClientAccept()
{
  m_asyncHandle = (uv_async_t*)malloc(sizeof(uv_async_t));
  m_asyncHandle->data = 0;
}

RpcClientAccept::~RpcClientAccept()
{
  if (m_asyncHandle->data == this)
    m_asyncHandle->data = 0;
  else
    free(m_asyncHandle);
}

static void async_close_cb(uv_handle_t* handle)
{
  free(handle);
}

static void sv_async_cb(uv_async_t* handle)
{
  if (handle->data == 0)
  {
    uv_close((uv_handle_t*)handle, async_close_cb);
    return;
  }

  RpcClientAccept* worker = (RpcClientAccept*)handle->data;
  worker->_OnAsync(handle);
}

int RpcClientAccept::_OnAsync(uv_async_t* handle)
{
  std::list<std::shared_ptr<ResultPacket>> result;
  m_resultList.clone(result);
  for (std::list<std::shared_ptr<ResultPacket>>::iterator it = result.begin(); it != result.end(); ++it)
  {
    std::shared_ptr<RpcTcpServerTransport> conn = it->get()->conn.lock();
    if (conn)
      conn->Send(YYRPC_PROTOCAL_RESULT, it->get()->data.c_str(), it->get()->data.length());
  }

  return 0;
}

int RpcClientAccept::Init(const std::string& ip, int port, uv_loop_t* loop)
{
  if (TcpAcceptor::Init(ip, port, loop) != 0)
    return -1;
  int r;
  r = uv_async_init(loop, m_asyncHandle, sv_async_cb);
  UV_CHECK_RET_1(uv_async_init, r);
  uv_unref((uv_handle_t*)m_asyncHandle);
  m_asyncHandle->data = this;
  return 0;
}

int RpcClientAccept::UnInit()
{
  return TcpAcceptor::UnInit();
}

TcpServerTransport* RpcClientAccept::NewTcpServerTransport()
{ 
  std::shared_ptr<RpcTcpServerTransport> conn = std::make_shared<RpcTcpServerTransport>(this);
  conn->SetConnected();
  RegisterTransport(conn);
  return conn.get();
}

int RpcClientAccept::RegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn)
{
  if (!conn)
    return -1;

  m_rsConn.push_back(conn);
  return 0;
}

int RpcClientAccept::UnRegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn)
{
  if (!conn)
    return -1;

  auto it = std::remove_if(m_rsConn.begin(), m_rsConn.end(), [&conn](std::shared_ptr<RpcTcpServerTransport>& p) { return p == conn; });
  m_rsConn.erase(it, m_rsConn.end());
  return 0;
}

bool RpcClientAccept::QueueSend(const std::weak_ptr<RpcTcpServerTransport>& conn, std::stringstream& s)
{
  std::shared_ptr<ResultPacket> result = std::make_shared<ResultPacket>(conn, s);
  m_resultList.push_back(result);
  uv_async_send(m_asyncHandle);
  return true;
}
