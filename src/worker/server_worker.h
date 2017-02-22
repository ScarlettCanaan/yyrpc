#ifndef YYRPC_SERVER_WORKER_H_
#define YYRPC_SERVER_WORKER_H_

#include "thread/thread_worker.h"
#include <memory>
#include "util/thread_safe_list.h"

struct CallPacket;

class ServerWorker : public ThreadWorker
{
public:
  ServerWorker();
  ~ServerWorker();

  int Init();
  int UnInit();
public:
  // ThreadWorker
  int _DoWork() override;
  int _OnAsync(uv_async_t* handle) override;
  int _OnClose(uv_handle_t* handle) override;

  int _OnIdle(uv_idle_t *handle);
public:
  int QueueTask(const std::shared_ptr<CallPacket>& task);
private:
  int DoTask(const std::shared_ptr<CallPacket>& task);
private:
  uv_idle_t m_idler;
  ThreadSafeList<std::shared_ptr<CallPacket>> m_taskList;
};


#endif  //! #ifndef YYRPC_SERVER_WORKER_H_
