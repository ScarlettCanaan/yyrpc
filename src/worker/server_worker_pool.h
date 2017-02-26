#ifndef __SERVER_WORKER_POOL_H_
#define __SERVER_WORKER_POOL_H_

#include <vector>
#include <memory>

class ServerWorker;
struct CalleePacket;

class ServerWorkerPool
{
public:
  static ServerWorkerPool& GetInstance();
public:
  int Init(size_t worker_num);
  int UnInit();
public:
  int QueuePacket(size_t groupId, const std::shared_ptr<CalleePacket>& task);
private:
  std::vector<std::shared_ptr<ServerWorker>> m_workers;
private:
  ServerWorkerPool();
  ~ServerWorkerPool();
};

#endif  //! #ifndef __SERVER_WORKER_POOL_H_
