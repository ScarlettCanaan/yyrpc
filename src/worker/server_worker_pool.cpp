#include "server_worker_pool.h"
#include "server_worker.h"

ServerWorkerPool& ServerWorkerPool::GetInstance()
{
  static ServerWorkerPool inst;
  return inst;
}

ServerWorkerPool::ServerWorkerPool()
{
  
}

ServerWorkerPool::~ServerWorkerPool()
{

}

int ServerWorkerPool::Init(size_t worker_num)
{
  if (!m_workers.empty())
    return -1;

  m_workers.reserve(worker_num);
  for (size_t i = 0; i < worker_num; ++i)
  {
    auto p = std::make_shared<ServerWorker>();
    if (p->Init() == 0)
      m_workers.push_back(p);
  }

  if (m_workers.size() != worker_num)
  {
    UnInit();
    return -1;
  }

  return 0;
}

int ServerWorkerPool::UnInit()
{
  for (auto it : m_workers)
    it->UnInit();
  m_workers.clear();
  return 0;
}

int ServerWorkerPool::QueuePacket(size_t groupId, const std::shared_ptr<CalleePacket>& task)
{
  if (m_workers.empty())
    return -1;

  size_t index = groupId % m_workers.size();
  return m_workers[index]->QueueTask(task);
}
