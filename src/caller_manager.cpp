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

CallerManager::CallerManager()
{
}

CallerManager::~CallerManager()
{
}

bool CallerManager::OnResult(std::thread::id id, std::shared_ptr<IAsyncResult> result)
{
  if (id != std::thread::id())
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    m_specThreadResult[id].insert(result);
    return true;
  }

  ClientWorkerPool::GetInstance().QueuePacket((int64_t)result.get() & 0xFF, result);
  return true;
}

bool CallerManager::PumpMessage()
{
  std::set<std::shared_ptr<IAsyncResult>> cloneSet;
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    cloneSet = m_specThreadResult[std::this_thread::get_id()];
    m_specThreadResult[std::this_thread::get_id()].clear();
  }

  for (auto it : cloneSet)
    it->run_callback();

  return !cloneSet.empty();
}
