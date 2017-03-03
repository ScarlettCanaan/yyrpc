#include "caller_manager.h"
#include "worker/client_worker_pool.h"
#include "util/util.h"
#include <iostream>
#include "error_def.h"
#include "transport/rpc_tcp_client_transport.h"
#include "async_result.h"

CallerManager& CallerManager::GetInstance()
{
  static CallerManager inst;
  return inst;
}

int CallerManager::Init(bool useFiber, size_t maxFiberNum)
{
  m_initThreadId = std::this_thread::get_id();
  m_useFiber = useFiber;
  m_maxFiberNum = maxFiberNum;
  return 0;
}

int CallerManager::UnInit()
{
  return 0;
}

CallerManager::CallerManager()
  : m_useFiber(false)
{
}

CallerManager::~CallerManager()
{
}

bool CallerManager::OnResult(std::thread::id id, std::shared_ptr<IAsyncResult> result)
{
  if (m_useFiber)
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    m_specThreadResult[m_initThreadId].push_back(result);
  }
  else if (id != std::thread::id())
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    m_specThreadResult[id].push_back(result);
  }
  else
  {
    ClientWorkerPool::GetInstance().QueuePacket((int64_t)result.get() & 0xFF, result);
  }
  return true;
}

bool CallerManager::PumpMessage()
{
  if (m_useFiber && m_initThreadId != std::this_thread::get_id())
  {
    LOG(ERROR) << "fiber must be called on the init thread.";
    return false;
  }

  std::thread::id id = std::this_thread::get_id();
  std::list<std::shared_ptr<IAsyncResult>> cloneSet;
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    cloneSet = m_specThreadResult[id];
    m_specThreadResult[id].clear();
  }

  for (auto it : cloneSet)
    it->run_callback();

  return !cloneSet.empty();
}

static void* fiber_work_thread(void* arg)
{
  CallerManager* server = (CallerManager*)arg;
  server->_OnFiberWork();

  return NULL;
}

void CallerManager::_OnFiberWork()
{
  st_thread_t t = st_thread_self();
  auto it = m_fiberThreadMap.find(t);
  if (it == m_fiberThreadMap.end())
  {
    LOG(ERROR) << "fiber thread not exist.";
    return;
  }
  it->second->func();
  m_fiberThreadMap.erase(t);
}

int CallerManager::CreateFiber(const std::function<int(void)>& f)
{
  std::shared_ptr<FunctionWrapper> wrap = std::make_shared<FunctionWrapper>();
  wrap->func = f;

  st_thread_t t = st_thread_create(fiber_work_thread, this, 0, 0);
  if (t == NULL)
  {
    LOG(ERROR) << "st_thread_create failed.";
    return -1;
  }

  m_fiberThreadMap[t] = wrap;
  return 0;
}

