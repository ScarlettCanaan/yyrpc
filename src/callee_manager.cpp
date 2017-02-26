#include "callee_manager.h"
#include "util/util.h"
#include <iostream>
#include "packet/packet.h"
#include "worker/server_worker_pool.h"
#include "callback.h"

CalleePacket::CalleePacket(const CallbackWrapper& wrapper_, 
  const std::shared_ptr<RpcTcpServerTransport>& endpoint_, 
  int64_t session_id_, 
  const std::string& method_name_,
  std::shared_ptr<msgpack::unpacker>& unp_)
{
  wrapper = wrapper_;
  endpoint = endpoint_;
  session_id = session_id_;
  method_name = method_name_;
  unp = unp_;
}

bool CalleePacket::run_impl() const
{
  if (!wrapper)
    return false;
  return wrapper.Invork(*unp, session_id, method_name, endpoint);
}

CalleeManager& CalleeManager::GetInstance()
{
  static CalleeManager inst;
  return inst;
}

CalleeManager::CalleeManager()
{
}

CalleeManager::~CalleeManager()
{
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

bool CalleeManager::OnCall(const std::shared_ptr<RpcTcpServerTransport>& endpoint,
  int64_t session_id, 
  const std::string& method_name, 
  std::shared_ptr<msgpack::unpacker> unp)
{
  const CallbackWrapper& wrapper = GetImpl(method_name);
  if (!wrapper)
    return false;

  std::shared_ptr<CalleePacket> pPacket 
    = std::make_shared<CalleePacket>(wrapper, endpoint, session_id, method_name, unp);

  std::thread::id id = wrapper.get_call_thread_id();
  if (id != std::thread::id())
  {
    std::lock_guard<std::mutex> l(m_caleeMutex);
    m_specThreadCallee[id].push_back(pPacket);
  }
  else
  {
    ServerWorkerPool::GetInstance().QueuePacket(0, pPacket);
  }

  return true;
}

bool CalleeManager::PumpMessage()
{
  std::list<std::shared_ptr<CalleePacket>> cloneList;
  {
    std::lock_guard<std::mutex> l(m_caleeMutex);
    cloneList = m_specThreadCallee[std::this_thread::get_id()];
    m_specThreadCallee[std::this_thread::get_id()].clear();
  }

  for (auto it : cloneList)
    it->run_impl();

  return !cloneList.empty();
}
