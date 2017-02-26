#include "listenpoint.h"
#include "rpc_client_accept.h"
#include "util/util.h"

ListenPoint::ListenPoint(const std::string& ip, int32_t port)
  : m_ip(ip), m_port(port)
{
  m_status = LPS_INIT;
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

  r = BindListen();
  m_status = (r == 0 ? LPS_LISTEN_SUCC : LPS_LISTEN_FAIL);
  UV_CHECK_RET_1(BindListen, r);

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
  OnClose();
  return 0;
}

int TcpListenPoint::BindListen()
{
  m_acceptor = std::make_shared<RpcClientAccept>();
  if (m_acceptor->Init(m_ip, m_port, &m_loop) != 0)
    return -1;
 
  return m_acceptor->Listen();
}

void TcpListenPoint::OnClose()
{
  m_acceptor.reset();
}

