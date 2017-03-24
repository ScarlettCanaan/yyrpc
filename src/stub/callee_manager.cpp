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

#include "callee_manager.h"
#include "../util/base_util.h"
#include <iostream>
#include "../packet/packet.h"
#include "../worker/server_worker_pool.h"
#include "callback.h"
#include "../error_def.h"
#include "fiber_callee_manager.h"
#include "../../proto/common_api.h"

_START_ORPC_NAMESPACE_

CalleePacket::CalleePacket(const std::string& method_name_, const CallbackWrapper& wrapper_, const CallbackInvorkParam& param_)
  : method_name(method_name_)
{
  wrapper = wrapper_;
  param = param_;
}

bool CalleePacket::RunImpl() const
{
  if (!wrapper)
    return false;
  return wrapper.Invork(param);
}

void CalleePacket::SendError(int32_t errorId) const
{
  param.transport->SendError(param.session_id, errorId);
}

CalleeManager& CalleeManager::GetInstance()
{
  static CalleeManager inst;
  return inst;
}

CalleeManager::CalleeManager()
  : m_eventDispatcher(nullptr)
{
}

CalleeManager::~CalleeManager()
{
}

int CalleeManager::Init()
{
  m_methodAuth.Init();
  return 0;
}

int CalleeManager::UnInit()
{
  m_methodAuth.UnInit();

#ifdef ORPC_USE_FIBER  
  m_fiberManager = nullptr;
#else
  m_serverWorkerPool = nullptr;
#endif  //! #ifdef ORPC_USE_FIBER
  return 0;
}

#ifdef ORPC_USE_FIBER  
bool CalleeManager::SetFiberInfo(bool useFiber, size_t maxFiberNum)
{
  if (m_fiberManager)
    return false;

  if (!useFiber)
    return true;

  m_fiberManager = std::make_shared<FiberCalleeManager>(maxFiberNum);
  return true;
}
#else
bool CalleeManager::SetThreadInfo(size_t threadNum)
{
  if (m_serverWorkerPool)
    return false;

  m_serverWorkerPool = std::make_shared<ServerWorkerPool>(threadNum);
  return true;
}
#endif  //! #ifdef ORPC_USE_FIBER

ListenPointWrapper CalleeManager::CreateListenPoint(const std::string& ip, int32_t port, int32_t flags)
{
  return m_listenPointManager.CreateListenPoint(ip, port, flags);
}

const CallbackWrapper& CalleeManager::GetImpl(const std::string& method_name) const
{
  static CallbackWrapper null;
  std::lock_guard<std::mutex> l(m_caleeMutex);
  auto it = m_callees.find(method_name);
  if (it == m_callees.end())
    return null;

  return it->second;
}

int CalleeManager::OnCall(const std::string& method_name, const CallbackInvorkParam& param)
{
  const CallbackWrapper& wrapper = GetImpl(method_name);
  if (!wrapper)
  {
    ORPC_LOG(ERROR) << "method:" << method_name << " not impl.";
    return ORPC_ERROR_CANT_FIND_METHOD;
  }

  if (!m_methodAuth.CheckAuth(method_name, param))
  {
    ORPC_LOG(ERROR) << "checkauth failed:" << method_name;
    return ORPC_ERROR_AUTH_FAILED;
  }

  std::shared_ptr<CalleePacket> pPacket
    = std::make_shared<CalleePacket>(method_name, wrapper, param);

#ifdef ORPC_USE_FIBER  
  if (m_fiberManager)
    return m_fiberManager->OnCall(pPacket);
  return 0;
#else
  if (wrapper.GetCallThreadId() != std::thread::id())
  {
    std::lock_guard<std::mutex> l(m_caleeMutex);
    m_specThreadCallee[wrapper.GetCallThreadId()].push_back(pPacket);
  }
  else if (m_serverWorkerPool)
  {
    m_serverWorkerPool->QueuePacket((int64_t)param.transport.get() && 0xFF, pPacket);
  }
  else
  {
    ORPC_LOG(ERROR) << "internal error:" << method_name;
    return ORPC_ERROR_UNKNOWN;
  }
  return 0;
#endif  //! #ifdef ORPC_USE_FIBER
}

bool CalleeManager::OnDispatchEvent(const std::string& method_name, const char* data, int32_t len)
{
  EventRequest event;
  event.eventName = std::move(method_name);
  event.eventData = std::string(data, len);
  m_eventRequests.push_back(event);
  return true;
}

bool CalleeManager::PumpMessage()
{
  PumpMonitorMessage();

  std::list<EventRequest> cloneEvent;
  m_eventRequests.clone(cloneEvent);
  for (auto it : cloneEvent)
  {
    if (m_eventDispatcher)
      m_eventDispatcher->OnEvent(it.eventName, it.eventData);
  }

#ifdef ORPC_USE_FIBER  
  if (m_fiberManager)
    return m_fiberManager->PumpMessage();
#endif  //! #ifdef ORPC_USE_FIBER

  std::thread::id id = std::this_thread::get_id();
  std::list<std::shared_ptr<CalleePacket>> cloneList;
  {
    std::lock_guard<std::mutex> l(m_caleeMutex);
    cloneList = m_specThreadCallee[id];
    m_specThreadCallee[id].clear();
  }

  for (auto it : cloneList)
    DoTask(it);

  return !cloneList.empty();
}

int CalleeManager::DoTask(const std::shared_ptr<CalleePacket>& task)
{
  if (!task)
    return 0;

  if (!task->RunImpl())
  {
    ORPC_LOG(ERROR) << "task run failed, method_name:" << task->method_name << " sessionid: " << task->param.session_id;
    task->SendError(ORPC_ERROR_BODY_FAILED);
  }

  return 0;
}

static void* fiber_work_thread(void* arg)
{
  CalleeManager* server = (CalleeManager*)arg;
  server->_OnFiberWork();

  return NULL;
}

void CalleeManager::OnHelloResponse(const std::list<ClientInfo>& clientInfos, const std::list<MethodItem>& methods)
{
  for (auto it : clientInfos)
    m_methodAuth.OnClientConnected(it.appId, it.token, it.privilege);
  for (auto it : methods)
    m_methodAuth.OnMethodInfo(it.method_name, it.nostate, it.privilege);
}

_END_ORPC_NAMESPACE_
