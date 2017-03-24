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

#ifndef ORPC_CALLEE_MANAGER_H_
#define ORPC_CALLEE_MANAGER_H_

#include "../build_config.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <list>
#include <thread>
#include <mutex>
#include "callback.h"
#include "../util/thread_safe_list.h"
#include "fiber_callee_manager.h"
#include "../worker/server_worker_pool.h"
#include "../listenpoint/listenpoint_manager.h"
#include "connection_monitor.h"
#include "method_auth.h"

struct ClientInfo;
struct MethodItem;

_START_ORPC_NAMESPACE_

class RpcTcpServerTransport;
class ICallback;

struct CalleePacket
{
public:
  CalleePacket(const std::string& method_name_, const CallbackWrapper& wrapper_, const CallbackInvorkParam& param_);

  bool RunImpl() const;

  void SendError(int32_t errorId) const;
public:
  std::string method_name;
  CallbackWrapper wrapper;
  CallbackInvorkParam param;
};

struct EventRequest
{
  std::string eventName;
  std::string eventData;
};

class EventDispatcher
{
public:
  virtual ~EventDispatcher() {}
public:
  virtual bool OnEvent(const std::string& eventName, const std::string& eventData) = 0;
};

class CalleeManager : public TransportManager
{
public:
  static CalleeManager& GetInstance();

public:
  int Init();
  int UnInit();
public:

#ifdef ORPC_USE_FIBER
  bool SetFiberInfo(bool useFiber, size_t maxFiberNum);
#else
  bool SetThreadInfo(size_t threadNum);
#endif  //! #ifdef ORPC_USE_FIBER

  ListenPointWrapper CreateListenPoint(const std::string& ip, int32_t port, int32_t flags);

  template <typename R, typename ... Args>
  bool BindApi(const std::string& method_name, const std::function<R(Args...)>& func, bool call_on_this_thread);

  template <typename R>
  bool BindApi(const std::string& method_name, const std::function<R(void)>& func, bool call_on_this_thread);

  int OnCall(const std::string& method_name, const CallbackInvorkParam& param);

  bool OnDispatchEvent(const std::string& method_name, const char* data, int32_t len);

  void SetEventDispatcher(EventDispatcher* disp) { m_eventDispatcher = disp; }

  void OnHelloResponse(const std::list<ClientInfo>& clientInfos, const std::list<MethodItem>& methods);

  bool NOTHROW PumpMessage();
  void NOTHROW _OnFiberWork();
private:
  int DoTask(const std::shared_ptr<CalleePacket>& task);
private:
  const CallbackWrapper& GetImpl(const std::string& method_name) const;
private:
  mutable std::mutex m_caleeMutex;
  std::map<std::thread::id, std::list<std::shared_ptr<CalleePacket>>> m_specThreadCallee;
  std::unordered_map<std::string, CallbackWrapper> m_callees;

  ThreadSafeList<EventRequest> m_eventRequests;

  EventDispatcher* m_eventDispatcher;

#ifdef ORPC_USE_FIBER  
  std::shared_ptr<FiberCalleeManager> m_fiberManager;
#else
  std::shared_ptr<ServerWorkerPool> m_serverWorkerPool;
#endif  //! #ifdef ORPC_USE_FIBER

  MethodAuth m_methodAuth;

  ListenPointManager m_listenPointManager;
private:
  friend class RpcApplication;
  CalleeManager();
  ~CalleeManager();
};

template <typename R, typename ... Args>
bool CalleeManager::BindApi(const std::string& method_name, const std::function<R(Args...)>& func,  bool call_on_this_thread)
{
  std::lock_guard<std::mutex> l(m_caleeMutex);
  if (m_callees.find(method_name) != m_callees.end())
    return false;

  std::shared_ptr<ICallback> p(
    new TRpcCallback<R(typename std::remove_cv<typename std::remove_reference<Args>::type>::type...)>(func));

  std::thread::id id;
  if (call_on_this_thread)
    id = std::this_thread::get_id();

  CallbackWrapper wrapper(p, id);
  m_callees[method_name] = wrapper;

  return true;
}

template <typename R>
bool CalleeManager::BindApi(const std::string& method_name, const std::function<R(void)>& func, bool call_on_this_thread)
{
  std::lock_guard<std::mutex> l(m_caleeMutex);
  if (m_callees.find(method_name) != m_callees.end())
    return false;

  std::shared_ptr<ICallback> p(
    new TRpcCallback<R>(func));

  std::thread::id id;
  if (call_on_this_thread)
    id = std::this_thread::get_id();

  CallbackWrapper wrapper(p, id);
  m_callees[method_name] = wrapper;

  return true;
}

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_CALLEE_MANAGER_H_
