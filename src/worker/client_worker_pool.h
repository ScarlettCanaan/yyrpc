#ifndef __CLIENT_WORKER_POOL_H_
#define __CLIENT_WORKER_POOL_H_

#include <vector>
#include <memory>

class ClientWorker;
class IAsyncResult;

class ClientWorkerPool
{
public:
  static ClientWorkerPool& GetInstance();
public:
  int Init(size_t worker_num);
  int UnInit();
public:
  int QueuePacket(size_t groupId, const std::shared_ptr<IAsyncResult>& task);
private:
  std::vector<std::shared_ptr<ClientWorker>> m_workers;
private:
  ClientWorkerPool();
  ~ClientWorkerPool();
};

#endif  //! #ifndef __CLIENT_WORKER_POOL_H_
