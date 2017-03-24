/*
* The MIT License (MIT)
*
* Copyright (c) 2017-2018 youjing@yy.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "endpoint.h"
#include "../util/base_util.h"
#include "../transport/rpc_tcp_client_transport.h"
#include "../marshal/unserialization.h"
#include "../stub/caller_manager.h"
#include "../error_def.h"
#include "../stub/callee_manager.h"
#include "../stub/callback.h"

#define HTTP_REQUEST_HEARDER " HTTP/1.1\r\n"\
"Content-Type: application/json; charset=utf-8\r\n"\
"Content-Length: "

_START_ORPC_NAMESPACE_

EndPoint::EndPoint(const std::string& ip, int32_t port, int32_t flags)
: m_ip(ip), m_port(port), m_transportflags(flags)
{
  m_tryCount = 0;
  m_nextTryTime = -1;
  m_maxTryCount = 6;

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

int EndPoint::Init(int32_t max_try_count)
{
  m_maxTryCount = max_try_count;

  Start();

  while (m_asyncHandle->data != this)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

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
    OnFinalFailed();
  }
  else
  {
    CheckCallTimeout();
    ReConnectIfNecessary();
  }
  return 0;
}

bool EndPoint::OnFinalFailed()
{
  std::map<int32_t, std::shared_ptr<ICallResult>> cloneResult; 
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    cloneResult = m_pendingResult;
    m_pendingResult.clear();
  }

  for (auto it : cloneResult)
  {
    if (it.second->SetException(ORPC_ERROR_CONNECT_FINAL_FAIL))
      CallerManager::GetInstance().OnResult(it.second->GetRunThreadId(), it.second);
  }

  return true;
}

bool EndPoint::CheckCallTimeout(int64_t cur_time, const std::shared_ptr<ICallResult>& result)
{
  if (!result->IsTimeouted(cur_time))
    return false;

  if (result->SetException(ORPC_ERROR_CALL_TIMEOUT))
    CallerManager::GetInstance().OnResult(result->GetRunThreadId(), result);

  return true;
}

bool EndPoint::CheckCallTimeout()
{
  std::lock_guard<std::mutex> l(m_resultMutex);
  if (m_pendingResult.empty())
    return true;

  int64_t cur_time = 0;
  GetTimeMillSecond(&cur_time);

  for (auto it = m_pendingResult.begin(); 
    it != m_pendingResult.end();
    it = m_pendingResult.erase(it))
  {
    std::shared_ptr<ICallResult>& result = it->second;
    if (!CheckCallTimeout(cur_time, result))
      break;
  }

  return true;
}

bool EndPoint::ReConnectIfNecessary()
{
  if (m_nextTryTime == -1)
    return true;

  int64_t cur_time = 0;
  GetTimeMillSecond(&cur_time);
  if (cur_time > m_nextTryTime)
    CreateChannel();

  return true;
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

  r = uv_timer_init(&m_loop, &m_timerLong);
  m_timerLong.data = this;
  uv_timer_start(&m_timerLong, timer_cb_long, 100, 100);

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
  PacketList clone_request;
  m_taskList.clone(clone_request);

  PacketList retry_request;
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
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    uv_async_send(m_asyncHandle);
  }

  return 0;
}

int EndPoint::_OnDestory(uv_handle_t* handle)
{
  uv_close((uv_handle_t*)&m_timerLong, 0);
  OnDestory();
  return 0;
}

CallResultWrapper<void> EndPoint::Call(uint64_t session_id, BufferStream& buff)
{
  if (session_id == -1)
    return CallResultWrapper <void>(ORPC_ERROR_MARSHAL_FAILED);

  std::shared_ptr<CallPacket> pPacket = std::make_shared<CallPacket>();
  pPacket->type = EPT_CALL;
  pPacket->buff = std::move(buff);
  if (IS_SYNC_CLIENT(m_transportflags))
  {
    DoTask(pPacket);
  }
  else
  {
    m_taskList.push_back(pPacket);
    uv_async_send(m_asyncHandle);
  }
  return CallResultWrapper <void>(ORPC_ERROR_SUCESS);
}

int EndPoint::Auth(BufferStream& buff)
{
  std::shared_ptr<CallPacket> pPacket = std::make_shared<CallPacket>();
  pPacket->type = EPT_AUTH;
  pPacket->buff = std::move(buff);
  if (IS_SYNC_CLIENT(m_transportflags))
  {
    DoTask(pPacket);
  }
  else
  {
    m_taskList.push_back(pPacket);
    uv_async_send(m_asyncHandle);
  }
  return 0;
}

TcpEndPoint::TcpEndPoint(const std::string& ip, int32_t port, int32_t flags)
  : EndPoint(ip, port, flags), RpcTcpClientTransport(flags)
{
  m_appId = -1;
  m_privilege = -1;
  m_sessionToken = -1;
  SetConnectMonitor(this);
}

int TcpEndPoint::DoTask(const std::shared_ptr<CallPacket>& task)
{
  if (!task || IsFinalFailed())
    return 0;

  if (GetConnectStatus() != CS_CONNECTED)
    return -1;

  int r = -1;
  if (IS_HTTP(m_transportflags))
  {
    std::string s = "GET ";
    s += task->type == EPT_CALL ? "/method_request" : "/auth";
    s += HTTP_REQUEST_HEARDER;
    s += NumberToString(task->buff.size());
    s += "\r\n\r\n";

    s.append(task->buff.data(), task->buff.size());
    r = Send(s.c_str(), s.length());
  }
  else 
  {
    r = Send(task->type == EPT_CALL ? ORPC_PROTOCAL_CALL : ORPC_PROTOCAL_AUTH, task->buff);
  }

  if (r < 0)
    Close(CR_PEER_CLOSED);

  return r;
}

void TcpEndPoint::CreateChannel()
{
  if (GetConnectStatus() == CS_CLOSED || GetConnectStatus() == CS_INIT)
    Connect(GetIp(), GetPort(), &m_loop);

  m_nextTryTime = -1;
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
    return ORPC_ERROR_HEADER_FAILED;

  unp.buffer_consumed(actual_read_size);

  int32_t session_id = 0;
  int32_t methodType;
  std::string methodName;
  int32_t error_id = 0;
  msgpack::object_handle o;
  if (!unp.next(o))
  {
    ORPC_LOG(ERROR) << "msgpack format error.";
    return ORPC_ERROR_HEADER_FAILED;
  }

  if (o.get().type != msgpack::type::MAP)
    return ORPC_ERROR_HEADER_FAILED;

  msgpack::object_map v = o.get().via.map;
  if (v.size == 0)
    return ORPC_ERROR_HEADER_FAILED;

  //header
  msgpack::object_kv* o1 = v.ptr;
  if (!UnserializationResultHeader(&o1->val, methodType, session_id, error_id, methodName))
    return ORPC_ERROR_HEADER_FAILED;

  msgpack::object_kv* o2 = nullptr;
  if (v.size >= 2)
    o2 = v.ptr + 1;
   
  if (methodType == 3/*"result"*/)
    return OnCallResult(session_id, error_id, o2);
  else if (methodType == 2/*"fire"*/)
    return OnFireEvent(methodName, o, o2);

  return 0;
}

int TcpEndPoint::OnCallResult(int32_t sessionId, int32_t errorId, const msgpack::object_kv* obj)
{
  std::shared_ptr<ICallResult> result; 
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    auto it = m_pendingResult.find(sessionId);
    if (it == m_pendingResult.end())
      return 0;
    result = it->second;
    m_pendingResult.erase(it);
  }

  if (errorId != 0)
  {
    if (result->SetException(errorId))
      CallerManager::GetInstance().OnResult(result->GetRunThreadId(), result);
  }
  else
  {
    if (obj && result->SetResult(&obj->val))
      CallerManager::GetInstance().OnResult(result->GetRunThreadId(), result);
  }

  return 0;
}

int TcpEndPoint::OnFireEvent(const std::string& eventName, msgpack::object_handle& o, const msgpack::object_kv* obj)
{
  if (!obj)
    return -1;

  std::shared_ptr<msgpack::object_handle> shared_obj = std::make_shared<msgpack::object_handle>(obj->val, std::move(o.zone()));
  CallbackInvorkParam param(shared_obj);
  return CalleeManager::GetInstance().OnCall(eventName, param);
}

int TcpEndPoint::OnConnected(bool bSucc)
{
  if (!bSucc)
    Close(0);

  TrySendAuth();
  return 0;
}

int TcpEndPoint::OnRequireClose(int reason)
{
  return 0;
}

int TcpEndPoint::OnAfterClose() 
{ 
  int64_t cur_time = 0;
  GetTimeMillSecond(&cur_time);
  m_nextTryTime = cur_time + std::pow(2, m_tryCount) * 2000;
  m_tryCount++;
  return 0;
}

bool TcpEndPoint::NeedWaitResult()
{
  std::lock_guard<std::mutex> l(m_resultMutex);
  return !m_pendingResult.empty();
}

void TcpEndPoint::SetAuth(int32_t appId, int32_t privilege, int64_t session_token)
{
  m_appId = appId;
  m_privilege = privilege;
  m_sessionToken = session_token;

  TrySendAuth();
}

bool TcpEndPoint::TrySendAuth()
{
  if (GetConnectStatus() != CS_CONNECTED || m_appId == -1)
    return false;

  BufferStream s;
  msgpack::packer<BufferStream> packer(s);
  packer.pack_map(1);
  packer.pack(0);
  SerializationAuthHeader(packer, m_appId, m_sessionToken, m_privilege);
  Auth(s);
  return true;
}

_END_ORPC_NAMESPACE_
