#include "callee_manager.h"
#include "util/util.h"
#include <iostream>
#include "packet/packet.h"
#include "worker/server_worker_pool.h"
#include "callback.h"
#include "error_def.h"

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
  : m_useFiber(false), m_maxFiberNum(1)
{
}

CalleeManager::~CalleeManager()
{
}

int CalleeManager::Init(bool useFiber, size_t maxFiberNum)
{
  m_initThreadId = std::this_thread::get_id();
  m_useFiber = useFiber;
  m_maxFiberNum = maxFiberNum;
  return 0;
}

int CalleeManager::UnInit()
{
  return 0;
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
  {
    LOG(ERROR) << "method:" << method_name << " not impl.";
    return endpoint->SendError(session_id, method_name, YYRPC_ERROR_CANT_FIND_METHOD);
  }

  std::shared_ptr<CalleePacket> pPacket 
    = std::make_shared<CalleePacket>(wrapper, endpoint, session_id, method_name, unp);

  if (m_useFiber)
  {
    std::lock_guard<std::mutex> l(m_caleeMutex);
    m_specThreadCallee[m_initThreadId].push_back(pPacket);
  }
  else if (wrapper.get_call_thread_id() != std::thread::id())
  {
    std::lock_guard<std::mutex> l(m_caleeMutex);
    m_specThreadCallee[wrapper.get_call_thread_id()].push_back(pPacket);
  }
  else
  {
    ServerWorkerPool::GetInstance().QueuePacket(0, pPacket);
  }

  return true;
}

bool CalleeManager::PumpMessage()
{
  std::thread::id id = std::this_thread::get_id();
  if (m_useFiber && m_initThreadId != id)
  {
    LOG(ERROR) << "fiber must be called on the init thread.";
    return false;
  }

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

  if (m_useFiber)
    return DoTaskInFiber(task);

  return DoTaskDirect(task);
}

static void* fiber_work_thread(void* arg)
{
  CalleeManager* server = (CalleeManager*)arg;
  server->_OnFiberWork();

  return NULL;
}

int CalleeManager::DoTaskDirect(const std::shared_ptr<CalleePacket>& task)
{
  if (!task->run_impl())
    LOG(ERROR) << "task run failed, method_name:" << task->method_name << " sessionid: " << task->session_id;

  return 0;
}

void CalleeManager::_OnFiberWork()
{
  st_thread_t t = st_thread_self();
  auto it = m_fiberThreadMap.find(t);
  if (it == m_fiberThreadMap.end())
  {
    LOG(ERROR) << "fiber thread not exist.";
    return;
  }

  it->second->run_impl();
  //m_fiberThreadMap may changed
  m_fiberThreadMap.erase(t);
}

int CalleeManager::DoTaskInFiber(const std::shared_ptr<CalleePacket>& task)
{
  if (m_fiberThreadMap.size() > m_maxFiberNum)
  {
    LOG(ERROR) << "fiber full, method_name:" << task->method_name << " sessionid: " << task->session_id;
    return -1;
  }

  st_thread_t t = st_thread_create(fiber_work_thread, this, 0, 0);
  if (t == NULL)
  {
    LOG(ERROR) << "st_thread_create failed, method_name:" << task->method_name << " sessionid: " << task->session_id;
    return -1;
  }

  m_fiberThreadMap[t] = task;
  return 0;
}
