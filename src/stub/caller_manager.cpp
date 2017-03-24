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

#include "caller_manager.h"
#include "../worker/client_worker_pool.h"
#include "../util/base_util.h"
#include <iostream>
#include "../error_def.h"
#include "../transport/rpc_tcp_client_transport.h"
#include "call_result.h"
#include "../../proto/common_api.h"
#include "../endpoint/endpoint_manager.h"

_START_ORPC_NAMESPACE_

CallerManager& CallerManager::GetInstance()
{
  static CallerManager inst;
  return inst;
}

int CallerManager::Init()
{
  return 0;
}

int CallerManager::UnInit()
{
#ifdef ORPC_USE_FIBER 
  m_fiberManager = 0;
#else
  m_clientWorkerPool = 0;
#endif  //! #ifdef ORPC_USE_FIBER
  return 0;
}

CallerManager::CallerManager()
{
  m_callPrivilege = -1;
}

CallerManager::~CallerManager()
{
}

#ifdef ORPC_USE_FIBER 
bool CallerManager::SetFiberInfo(bool useFiber, size_t maxFiberNum)
{
  if (m_fiberManager)
    return false;

  if (!useFiber)
    return true;

  m_fiberManager = std::make_shared<FiberCallerManager>(maxFiberNum);
  return true;
}
#else
bool CallerManager::SetThreadInfo(size_t threadNum)
{
  if (m_clientWorkerPool)
    return false;

  m_clientWorkerPool = std::make_shared<ClientWorkerPool>(threadNum);
  return true;
}
#endif  //! #ifdef ORPC_USE_FIBER

EndPointWrapper CallerManager::QueryEndPoint(const std::string& api) const
{
  return m_endPointManager.QueryEndPoint(api);
}

EndPointWrapper CallerManager::CreateEndPoint(const std::string& ip, int32_t port, int32_t flags)
{
  return m_endPointManager.CreateEndPoint(ip, port, flags);
}

bool CallerManager::CheckEndpointAllReady(uint32_t timeoutMs)
{
  return m_endPointManager.CheckEndpointAllReady(timeoutMs);
}

void CallerManager::OnHelloResponse(int32_t appId, const HelloClientResponse& helloRsp)
{
  m_callPrivilege = helloRsp.privilege;
  for (auto it : helloRsp.services)
    m_endPointManager.RegisterApi(appId, m_callPrivilege, it);
}

bool CallerManager::OnResult(std::thread::id id, std::shared_ptr<ICallResult> result)
{
  if (!result->HasCallback())
    return true;

#ifdef ORPC_USE_FIBER      
  if (m_fiberManager)
    return m_fiberManager->OnResult(id, result);
  return true;
#else
  if (id != std::thread::id())
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    m_specThreadResult[id].push_back(result);
  }
  else if (m_clientWorkerPool)
  {
    m_clientWorkerPool->QueuePacket((int64_t)result.get() & 0xFF, result);
  }
  else
  {
    ORPC_LOG(ERROR) << "CallerManager internal error.";
    return false;
  }
  return true;
#endif  //! #ifdef ORPC_USE_FIBER
}

bool CallerManager::PumpMessage()
{
#ifdef ORPC_USE_FIBER 
  if (m_fiberManager)
    return m_fiberManager->PumpMessage();
  return true;
#else
  std::thread::id id = std::this_thread::get_id();
  std::list<std::shared_ptr<ICallResult>> cloneSet;
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    cloneSet = m_specThreadResult[id];
    m_specThreadResult[id].clear();
  }

  for (auto it : cloneSet)
    it->RunCallback();

  return !cloneSet.empty();
#endif  //! #ifdef ORPC_USE_FIBER
}

#ifdef ORPC_USE_FIBER 
int CallerManager::CreateFiber(const std::function<int(void)>& f)
{
  if (!m_fiberManager)
    return -1;

  return m_fiberManager->CreateFiber(f);
}
#endif  //! #ifdef ORPC_USE_FIBER

_END_ORPC_NAMESPACE_
