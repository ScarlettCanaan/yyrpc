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

#include "fiber_callee_manager.h"

#ifdef ORPC_USE_FIBER

#include "util/base_util.h"
#include <iostream>
#include "packet/packet.h"
#include "worker/server_worker_pool.h"
#include "callback.h"
#include "error_def.h"
#include "callee_manager.h"

_START_ORPC_NAMESPACE_

FiberCalleeManager::FiberCalleeManager(size_t maxFiberNum)
{
  m_maxFiberNum = maxFiberNum;
  m_initThreadId = std::this_thread::get_id();
}

FiberCalleeManager::~FiberCalleeManager()
{

}

int FiberCalleeManager::OnCall(const std::shared_ptr<CalleePacket>& packet)
{
  std::lock_guard<std::mutex> l(m_caleeMutex);
  m_specThreadCallee[m_initThreadId].push_back(packet);
  return 0;
}

bool FiberCalleeManager::PumpMessage()
{
  std::thread::id id = std::this_thread::get_id();
  if (m_initThreadId != id)
  {
    ORPC_LOG(ERROR) << "fiber must be called on the init thread.";
    return false;
  }

  std::list<std::shared_ptr<CalleePacket>> cloneList;
  {
    std::lock_guard<std::mutex> l(m_caleeMutex);
    cloneList = m_specThreadCallee[id];
    m_specThreadCallee[id].clear();
  }

  for (auto it : cloneList)
  {
    DoTask(it);
  }

  return !cloneList.empty();
}


static void* fiber_work_thread(void* arg)
{
  FiberCalleeManager* server = (FiberCalleeManager*)arg;
  server->_OnFiberWork();

  return NULL;
}

void FiberCalleeManager::_OnFiberWork()
{
  st_thread_t t = st_thread_self();
  auto it = m_fiberThreadMap.find(t);
  if (it == m_fiberThreadMap.end())
  {
    ORPC_LOG(ERROR) << "fiber thread not exist.";
    return;
  }

  if (!it->second->RunImpl())
  {
    ORPC_LOG(ERROR) << "task run failed, method_name:" << it->second->method_name << " sessionid: " << it->second->param.session_id;
    it->second->SendError(ORPC_ERROR_BODY_FAILED);
  }

  m_fiberThreadMap.erase(t);
}

int FiberCalleeManager::DoTask(const std::shared_ptr<CalleePacket>& task)
{
  if (m_fiberThreadMap.size() > m_maxFiberNum)
  {
    ORPC_LOG(ERROR) << "fiber full, method_name:" << task->method_name << " sessionid: " << task->param.session_id;
    return -1;
  }

  st_thread_t t = st_thread_create(fiber_work_thread, this, 0, 0);
  if (t == NULL)
  {
    ORPC_LOG(ERROR) << "st_thread_create failed, method_name:" << task->method_name << " sessionid: " << task->param.session_id;
    return -1;
  }

  m_fiberThreadMap[t] = task;
  return 0;
}

_END_ORPC_NAMESPACE_

#endif  //! #ifdef ORPC_USE_FIBER
