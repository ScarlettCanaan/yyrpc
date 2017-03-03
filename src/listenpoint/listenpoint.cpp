#include "listenpoint.h"
#include "rpc_client_accept.h"
#include "util/util.h"

ListenPoint::ListenPoint(const std::string& ip, int32_t port, MethodProtocol mProtocal)
  : m_ip(ip), m_port(port), m_methodProtocal(mProtocal)
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

  r = BindListen(m_methodProtocal);
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

int ListenPoint::_OnDestory(uv_handle_t* handle)
{
  OnDestory();
  return 0;
}

int TcpListenPoint::BindListen(MethodProtocol mProtocal)
{
  m_acceptor = std::make_shared<RpcClientAccept>(mProtocal);
  if (m_acceptor->Init(m_ip, m_port, &m_loop) != 0)
    return -1;
 
  return m_acceptor->Listen();
}

void TcpListenPoint::OnDestory()
{
  m_acceptor.reset();
}

