#include "caller_manager.h"
#include "worker/client_worker_pool.h"
#include "util/util.h"
#include <iostream>

CallPacket::CallPacket(const std::shared_ptr<EndPoint>& endpoint, uint32_t session_id, std::stringstream& s)
{
  m_endpoint = endpoint;
  m_sessionId = session_id;
  m_data = s.str();
  GetTimeSecond(&m_callTime);
}

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

void CallerManager::QueuePacket(const std::shared_ptr<CallPacket>& pPacket)
{
  ClientWorkerPool::GetInstance().QueuePacket(0, pPacket);
}
