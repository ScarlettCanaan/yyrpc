#ifndef YY_CALLEE_MANAGER_H_
#define YY_CALLEE_MANAGER_H_

#include <string>
#include <memory>
#include <unordered_map>
#include <list>
#include <thread>
#include <mutex>
#include "msgpack.hpp"
#include "callback.h"
#include "st.h"
#include "build_config.h"

class RpcTcpServerTransport;
class ICallback;

struct CalleePacket
{
public:
  CalleePacket(const CallbackWrapper& wrapper_, 
    const std::shared_ptr<RpcTcpServerTransport>& endpoint_, 
    int64_t session_id_, 
    const std::string& method_name_, 
    std::shared_ptr<msgpack::unpacker>& unp_);

  bool run_impl() const;

public:
  CallbackWrapper wrapper;
  int64_t session_id;
  std::string method_name;
  std::shared_ptr<msgpack::unpacker> unp;
  std::shared_ptr<RpcTcpServerTransport> endpoint;
};

class CalleeManager
{
public:
  static CalleeManager& GetInstance();

  int Init(bool useFiber, size_t maxFiberNum);
  int UnInit();
public:
  template <typename R, typename ... Args>
  bool BindApi(const std::string& method_name, 
    const std::function<R(Args...)>& func, 
    bool call_on_this_thread);

  bool OnCall(const std::shared_ptr<RpcTcpServerTransport>& endpoint,
    int64_t session_id, 
    const std::string& method_name, 
    std::shared_ptr<msgpack::unpacker> unp);

  bool NOTHROW PumpMessage();
  void NOTHROW _OnFiberWork();
private:
  int DoTask(const std::shared_ptr<CalleePacket>& task);
  int NOTHROW DoTaskInFiber(const std::shared_ptr<CalleePacket>& task);
  int DoTaskDirect(const std::shared_ptr<CalleePacket>& task);
private:
  const CallbackWrapper& GetImpl(const std::string& method_name) const;
private:
  mutable std::mutex m_caleeMutex;
  std::unordered_map<std::string, CallbackWrapper> m_callees;

  std::map<std::thread::id, std::list<std::shared_ptr<CalleePacket>>> m_specThreadCallee;

  std::thread::id m_initThreadId;
  bool m_useFiber;
  size_t m_maxFiberNum;

  std::unordered_map<st_thread_t, std::shared_ptr<CalleePacket>> m_fiberThreadMap;
private:
  CalleeManager();
  ~CalleeManager();
};

template <typename R, typename ... Args>
bool CalleeManager::BindApi(const std::string& method_name, 
  const std::function<R(Args...)>& func, 
  bool call_on_this_thread)
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

#endif  //! #ifndef YY_CALL_MANAGER_H_
