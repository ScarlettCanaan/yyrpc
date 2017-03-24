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

#include "fiber_caller_manager.h"

#ifdef ORPC_USE_FIBER

#include "worker/client_worker_pool.h"
#include "util/base_util.h"
#include <iostream>
#include "error_def.h"
#include "transport/rpc_tcp_client_transport.h"
#include "call_result.h"
#include "../proto/common_api.h"
#include "endpoint/endpoint_manager.h"

_START_ORPC_NAMESPACE_

FiberCallerManager::FiberCallerManager(size_t maxFiberNum)
{
  m_maxFiberNum = maxFiberNum;
  m_initThreadId = std::this_thread::get_id();
}

FiberCallerManager::~FiberCallerManager()
{

}

bool FiberCallerManager::OnResult(std::thread::id id, std::shared_ptr<ICallResult> result)
{
  std::lock_guard<std::mutex> l(m_resultMutex);
  m_specThreadResult[m_initThreadId].push_back(result);

  return true;
}

bool FiberCallerManager::PumpMessage()
{
  if (m_initThreadId != std::this_thread::get_id())
  {
    ORPC_LOG(ERROR) << "fiber must be called on the init thread.";
    return false;
  }

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
}

static void* fiber_work_thread(void* arg)
{
  FiberCallerManager* server = (FiberCallerManager*)arg;
  server->_OnFiberWork();

  return NULL;
}

void FiberCallerManager::_OnFiberWork()
{
  st_thread_t t = st_thread_self();
  auto it = m_fiberThreadMap.find(t);
  if (it == m_fiberThreadMap.end())
  {
    ORPC_LOG(ERROR) << "fiber thread not exist.";
    return;
  }
  it->second->func();
  m_fiberThreadMap.erase(t);
}

int FiberCallerManager::CreateFiber(const std::function<int(void)>& f)
{
  if (m_fiberThreadMap.size() > m_maxFiberNum)
  {
    ORPC_LOG(ERROR) << "fiber full.";
    return -1;
  }

  std::shared_ptr<FunctionWrapper> wrap = std::make_shared<FunctionWrapper>();
  wrap->func = f;

  st_thread_t t = st_thread_create(fiber_work_thread, this, 0, 0);
  if (t == NULL)
  {
    ORPC_LOG(ERROR) << "st_thread_create failed.";
    return -1;
  }

  m_fiberThreadMap[t] = wrap;
  return 0;
}

_END_ORPC_NAMESPACE_

#endif  //! #ifdef ORPC_USE_FIBER
