#include "listenpoint.h"
#include "rpc_client_accept.h"
#include "util/util.h"

ListenPoint::ListenPoint(const std::string& ip, int32_t port, TransportProtocol protocal)
  : m_ip(ip), m_port(port), m_protocal(protocal)
{
  
}

int ListenPoint::BindTcpListen()
{
  m_acceptor = std::make_shared<RpcClientAccept>();
  m_acceptor->Init(m_ip, m_port, &m_loop);
  m_acceptor->Listen();

  return -1;
}

int ListenPoint::Init()
{
  Start();

  return 0;
}

int ListenPoint::UnInit()
{
  Stop();
  Join();

  return 0;
}

int ListenPoint::_DoWork()
{
  _PrepareWork();

  int r;

  if (m_protocal == TP_TCP)
    BindTcpListen();

  r = uv_run(&m_loop, UV_RUN_DEFAULT);
  UV_CHECK_RET_1(uv_run, r);

  r = uv_loop_close(&m_loop);
  UV_CHECK_RET_1(uv_loop_close, r);

  return 0;
}

int ListenPoint::_OnAsync(uv_async_t* handle)
{
  ThreadWorker::_OnAsync(handle);
  return 0;
}

int ListenPoint::_OnClose(uv_handle_t* handle)
{
  m_acceptor.reset();
  return 0;
}

