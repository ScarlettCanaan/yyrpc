#ifndef __CLIENT_WORKER_H_
#define __CLIENT_WORKER_H_

#include "thread/thread_worker.h"
#include <memory>
#include "util/thread_safe_list.h"

class IAsyncResult;

class ClientWorker : public ThreadWorker
{
public:
  ClientWorker();
  ~ClientWorker();

  int Init();
  int UnInit();
public:
  // ThreadWorker
  int _DoWork() override;
  int _OnAsync(uv_async_t* handle) override;
  int _OnDestory(uv_handle_t* handle) override;

  int _OnIdle(uv_idle_t *handle);
public:
  int QueueTask(const std::shared_ptr<IAsyncResult>& task);
private:
  int DoTask(const std::shared_ptr<IAsyncResult>& task);
private:
  uv_idle_t m_idler;
  ThreadSafeList<std::shared_ptr<IAsyncResult>> m_taskList;
};

#endif  //! #ifndef __CLIENT_WORKER_H_
