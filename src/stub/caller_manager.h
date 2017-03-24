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

#ifndef ORPC_CALLER_MANAGER_H_
#define ORPC_CALLER_MANAGER_H_

#include "../build_config.h"
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <functional>
#include <unordered_map>
#include "call_result.h"
#include "fiber_caller_manager.h"
#include "../worker/client_worker_pool.h"
#include "../endpoint/endpoint_manager.h"

struct HelloClientResponse;

_START_ORPC_NAMESPACE_

class ICallResult;

class CallerManager
{
public:
  static CallerManager& GetInstance();
public:
  int Init();
  int UnInit();
public:

#ifdef ORPC_USE_FIBER 
  bool SetFiberInfo(bool useFiber, size_t maxFiberNum);
#else
  bool SetThreadInfo(size_t threadNum);
#endif  //! #ifdef ORPC_USE_FIBER

  void SetNameCenter(const EndPointWrapper& wrapper) { m_nameCenter = wrapper; }
  const EndPointWrapper& GetNameCenter() const { return m_nameCenter; }

  EndPointWrapper QueryEndPoint(const std::string& api) const;
  EndPointWrapper CreateEndPoint(const std::string& ip, int32_t port, int32_t flags);
  bool CheckEndpointAllReady(uint32_t timeoutMs = -1);

  void OnHelloResponse(int32_t appId, const HelloClientResponse& helloRsp);

  bool OnResult(std::thread::id id, std::shared_ptr<ICallResult> result);

  bool PumpMessage();

#ifdef ORPC_USE_FIBER 
  template<typename R>
  CallResultWrapper<R> NOTHROW TryBlock(CallResultWrapper<R>& wrapper);

  CallResultWrapper<void> NOTHROW TryBlock(CallResultWrapper<void>& wrapper) { return wrapper; }

  int NOTHROW CreateFiber(const std::function<int(void)>& f);
#endif  //! #ifdef ORPC_USE_FIBER

private:
  int32_t m_callPrivilege;

#ifdef ORPC_USE_FIBER 
  std::shared_ptr<FiberCallerManager> m_fiberManager;
#else
  std::shared_ptr<ClientWorkerPool>   m_clientWorkerPool;

  mutable std::mutex m_resultMutex;
  std::map<std::thread::id, std::list<std::shared_ptr<ICallResult>>> m_specThreadResult;
#endif  //! #ifdef ORPC_USE_FIBER

  EndPointManager  m_endPointManager;

  EndPointWrapper m_nameCenter;
private:
  friend class RpcApplication;
  CallerManager();
  ~CallerManager();
};

#ifdef ORPC_USE_FIBER 
template<typename R>
CallResultWrapper<R> CallerManager::TryBlock(CallResultWrapper<R>& wrapper)
{
  if (!m_fiberManager)
    return wrapper;

  return m_fiberManager->TryBlock(wrapper);
}
#endif  //! #ifdef ORPC_USE_FIBER

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_CALLER_MANAGER_H_
