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

EndPoint::EndPoint(const std::string& ip, int32_t port, MethodProtocol mProtocal)
  : m_ip(ip), m_port(port), m_methodProtocal(mProtocal)
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

bool EndPoint::CheckCallTimeout(int64_t cur_time, const std::shared_ptr<IAsyncResult>& result)
{
  if (!result->is_timeouted(cur_time))
    return false;
  if (result->set_exception(YYRPC_ERROR_CALL_TIMEOUT))
    CallerManager::GetInstance().OnResult(result->get_run_thread_id(), result);

  return true;
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
    std::shared_ptr<IAsyncResult>& result = it->second;
    if (!CheckCallTimeout(cur_time, result))
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
  {
    m_nextTryTime = -1;
    CreateChannel();
  }

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

int EndPoint::_OnDestory(uv_handle_t* handle)
{
  uv_close((uv_handle_t*)&timer_long, 0);
  OnDestory();
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

  int r = -1;
  if (m_methodProtocal == MP_HTTP)
  {
    std::string s = "GET /method_request HTTP/1.1\r\n"
      "Content-Type: application/json; charset=utf-8\r\n"
      "Content-Length: ";
    s += NumberToString(task->m_data.length());
    s += "\r\n\r\n";
    s.append(task->m_data.c_str(), task->m_data.length());
    r = Send(s.c_str(), s.length());
  }
  else
  {
    r = Send(YYRPC_PROTOCAL_CALL, task->m_data.c_str(), task->m_data.length());
  }

  if (r < 0)
    Close(CR_PEER_CLOSED);
  return r;
}

void TcpEndPoint::CreateChannel()
{
  if (GetConnectStatus() == CS_CLOSED || GetConnectStatus() == CS_INIT)
    Connect(GetIp(), GetPort(), &m_loop);
}

void TcpEndPoint::OnDestory()
{

}

int TcpEndPoint::OnProcessResult(const char* data, int32_t len)
{
  m_tryCount = 0;

  std::string s(data, len);

  msgpack::unpacker unp;
  unp.reserve_buffer(s.size());
  std::stringstream input(s);
  std::size_t actual_read_size = input.readsome(unp.buffer(), s.size());
  if (actual_read_size != s.size())
    return -1;

  unp.buffer_consumed(actual_read_size);

  int32_t session_id = 0;
  std::string msgType;
  int32_t error_id = 0;
  if (!unserialization_result_header(unp, msgType, session_id, error_id))
    return -1;

  if (msgType == "/method_result")
    return OnCallResult(session_id, error_id, unp);
  else if (msgType == "/fire_event")
    return OnFireEvent(session_id, error_id, unp);

  return 0;
}

int TcpEndPoint::OnCallResult(int32_t session_id, int32_t error_id, msgpack::unpacker& unp)
{
  std::shared_ptr<IAsyncResult> result;
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    auto it = m_pendingResult.find(session_id);
    if (it == m_pendingResult.end())
      return 0;
    result = it->second;
    m_pendingResult.erase(it);
  }

  if (error_id != 0)
  {
    if (result->set_exception(error_id))
      CallerManager::GetInstance().OnResult(result->get_run_thread_id(), result);
  }
  else if (result->set_result(unp))
  {
    CallerManager::GetInstance().OnResult(result->get_run_thread_id(), result);
  }

  return 0;
}

int TcpEndPoint::OnFireEvent(int32_t session_id, int32_t error_id, msgpack::unpacker& unp)
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
  m_nextTryTime = cur_time + std::pow(2, m_tryCount) * 1000;
  m_tryCount++;
  return 0;
}
