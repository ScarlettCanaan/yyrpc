#include "endpoint.h"
#include "util/util.h"
#include "msgpack.hpp"
#include "transport/rpc_tcp_client_transport.h"
#include "unserialization.h"
#include "caller_manager.h"
#include "error_def.h"

CallerPacket::CallerPacket(uint64_t session_id, std::stringstream& s)
{
  m_sessionId = session_id;
  m_data = s.str();
}

EndPoint::EndPoint(const std::string& ip, int32_t port)
  : m_ip(ip), m_port(port)
{
  m_tryCount = 0;
  m_nextTryTime = -1;

  m_asyncHandle = (uv_async_t*)malloc(sizeof(uv_async_t));
  m_asyncHandle->data = 0;
}

EndPoint::~EndPoint()
{
  if (m_asyncHandle->data == this)
    m_asyncHandle->data = 0;
  else
    free(m_asyncHandle);
}

int EndPoint::Init()
{
  Start();

  while (m_asyncHandle->data != this)
    uv_sleep(1);

  return 0;
}

int EndPoint::UnInit()
{
  Stop();
  Join();

  return 0;
}

static void timer_cb_long(uv_timer_t* handle)
{
  EndPoint* worker = (EndPoint*)handle->data;
  worker->_OnTimerLong(handle);
}

int EndPoint::_OnTimerLong(uv_timer_t *handle)
{
  if (IsFinalFailed())
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    for (auto it : m_pendingResult)
      it.second->set_exception(YYRPC_ERROR_CONNECT_FINAL_FAIL);
    m_pendingResult.clear();
  }
  else
  {
    CheckCallTimeout();
    ReConnectIfNecessary();
  }
  return 0;
}

int EndPoint::CheckCallTimeout()
{
  std::lock_guard<std::mutex> l(m_resultMutex);
  if (m_pendingResult.empty())
    return 0;

  int64_t cur_time = 0;
  GetTimeMillSecond(&cur_time);

  for (auto it = m_pendingResult.begin(); it != m_pendingResult.end(); it = m_pendingResult.erase(it))
  {
    if (!it->second->check_timeout(cur_time))
      break;
  }

  return 0;
}

int EndPoint::ReConnectIfNecessary()
{
  if (m_nextTryTime == -1)
    return 0;

  int64_t cur_time = 0;
  GetTimeMillSecond(&cur_time);
  if (cur_time > m_nextTryTime)
    CreateChannel();

  return 0;
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

  EndPoint* worker = (EndPoint*)handle->data;
  worker->_OnAsyncWork(handle);
}

int EndPoint::_DoWork()
{
  _PrepareWork();

  int r;

  CreateChannel();

  r = uv_timer_init(&m_loop, &timer_long);
  timer_long.data = this;
  uv_timer_start(&timer_long, timer_cb_long, 100, 100);

  r = uv_async_init(&m_loop, m_asyncHandle, sv_async_cb);
  UV_CHECK_RET_1(uv_async_init, r);
  uv_unref((uv_handle_t*)m_asyncHandle);
  m_asyncHandle->data = this;

  r = uv_run(&m_loop, UV_RUN_DEFAULT);
  UV_CHECK_RET_1(uv_run, r);

  r = uv_loop_close(&m_loop);
  UV_CHECK_RET_1(uv_loop_close, r);

  return 0;
}

int EndPoint::_OnAsync(uv_async_t* handle)
{
  ThreadWorker::_OnAsync(handle);
  return 0;
}

int EndPoint::_OnAsyncWork(uv_async_t* handle)
{
  std::list<std::shared_ptr<CallerPacket>> clone_request;
  m_taskList.clone(clone_request);

  std::list<std::shared_ptr<CallerPacket>> retry_request;
  auto it = clone_request.begin();
  for (; it != clone_request.end(); ++it)
  {
    if (DoTask(*it) != 0)
      retry_request.push_back(*it);
  }

  auto retry_it = retry_request.rbegin();
  for (; retry_it != retry_request.rend(); ++retry_it)
    m_taskList.push_front(*retry_it);

  if (!retry_request.empty())
  {
    uv_sleep(1);
    uv_async_send(m_asyncHandle);
  }

  return 0;
}

int EndPoint::_OnClose(uv_handle_t* handle)
{
  uv_close((uv_handle_t*)&timer_long, 0);
  OnClose();
  return 0;
}

bool EndPoint::IsFinalFailed() const
{
  return m_tryCount >= 6;
}

void EndPoint::ResetFail()
{
  m_tryCount = 0;
}

int TcpEndPoint::DoTask(const std::shared_ptr<CallerPacket>& task)
{
  if (!task || IsFinalFailed())
    return 0;

  if (GetConnectStatus() != CS_CONNECTED)
    return -1;

  int r = Send(YYRPC_PROTOCAL_CALL, task->m_data.c_str(), task->m_data.length());
  if (r < 0)
    Close(CR_PEER_CLOSED);
  else
    m_tryCount = 0;
  return r;
}

void TcpEndPoint::CreateChannel()
{
  if (GetConnectStatus() == CS_CLOSED || GetConnectStatus() == CS_INIT)
    Connect(GetIp(), GetPort(), &m_loop);
}

void TcpEndPoint::OnClose()
{
  
}

int TcpEndPoint::OnProcessResult(const Packet* rawPacket)
{
  std::string s(rawPacket->getBodyData(), rawPacket->getBodyLength());

  msgpack::unpacker unp;
  unp.reserve_buffer(s.size());
  std::stringstream input(s);
  std::size_t actual_read_size = input.readsome(unp.buffer(), s.size());
  if (actual_read_size != s.size())
    return -1;

  unp.buffer_consumed(actual_read_size);

  int32_t session_id = 0;
  std::string method_name;
  if (!unserialization_header(unp, session_id, method_name))
    return -1;

  std::shared_ptr<IAsyncResult> result;
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    auto it = m_pendingResult.find(session_id);
    if (it == m_pendingResult.end())
      return 0;
    result = it->second;
    m_pendingResult.erase(it);
  }

  msgpack::object_handle o;
  if (!unp.next(o))
    return -1;

  int32_t error_id = o.get().as<int32_t>();
  if (error_id != 0)
  {
    result->set_exception(error_id);
  }
  else if (!result->set_result(unp))
  {
    return -1;
  }

  return 0;
}

int TcpEndPoint::OnProcessEvent(const Packet* rawPacket)
{
  return 0;
}

int TcpEndPoint::OnConnected(bool bSucc)
{
  if (!bSucc)
    Close(0);
  return 0;
}

int TcpEndPoint::OnAfterClose()
{ 
  int64_t cur_time = 0;
  GetTimeMillSecond(&cur_time);
  if (m_nextTryTime == -1)
    m_nextTryTime = cur_time + 1000;
  else
    m_nextTryTime = cur_time + std::pow(2, m_tryCount) * 1000;

  m_tryCount++;
  return 0;
}
