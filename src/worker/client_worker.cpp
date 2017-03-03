#include "client_worker.h"
#include "util/util.h"
#include "async_result.h"

ClientWorker::ClientWorker()
{

}

ClientWorker::~ClientWorker()
{

}

int ClientWorker::Init()
{
  Start();

  return 0;
}

int ClientWorker::UnInit()
{
  Stop();

  std::shared_ptr<IAsyncResult> null;
  QueueTask(null);

  Join();

  return 0;
}

static void idle_cb(uv_idle_t *handle) {
  ClientWorker* worker = (ClientWorker*)handle->data;
  worker->_OnIdle(handle);
}

int ClientWorker::_DoWork()
{
  _PrepareWork();

  int r;

  uv_idle_init(&m_loop, &m_idler);
  m_idler.data = this;
  uv_idle_start(&m_idler, idle_cb);

  r = uv_run(&m_loop, UV_RUN_DEFAULT);
  UV_CHECK_RET_1(uv_run, r);

  r = uv_loop_close(&m_loop);
  UV_CHECK_RET_1(uv_loop_close, r);

  return 0;
}

int ClientWorker::_OnAsync(uv_async_t* handle)
{
  ThreadWorker::_OnAsync(handle);
  return 0;
}

int ClientWorker::_OnDestory(uv_handle_t* handle)
{
  uv_close((uv_handle_t*)&m_idler, NULL);
  return 0;
}

int ClientWorker::_OnIdle(uv_idle_t *handle)
{
  std::list<std::shared_ptr<IAsyncResult>> clone_request;
  m_taskList.clone(clone_request);

  std::list<std::shared_ptr<IAsyncResult>> retry_request;

  auto it = clone_request.begin();
  for (; it != clone_request.end(); ++it)
  {
    if (DoTask(*it) != 0)
      retry_request.push_back(*it);
  }

  auto retry_it = retry_request.rbegin();
  for (; retry_it != retry_request.rend(); ++retry_it)
    m_taskList.push_front(*retry_it);

  m_taskList.try_wait(std::chrono::milliseconds(50));
  return 0;
}

int ClientWorker::QueueTask(const std::shared_ptr<IAsyncResult>& task)
{
  m_taskList.push_back(task);
  return 0;
}

int ClientWorker::DoTask(const std::shared_ptr<IAsyncResult>& task)
{
  if (!task)
    return 0;

  task->run_callback();

  return 0;
}
