/*
* The MIT License (MIT)
*
* Copyright (c) 2017-2018 youjing@yy.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "thread_worker.h"
#include <assert.h>
#include "../util/base_util.h"

#ifndef WIN32
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#endif

_START_ORPC_NAMESPACE_

ThreadWorker::ThreadWorker()
{
  m_threadId = (uv_thread_t)-1;
}

ThreadWorker::~ThreadWorker()
{

}

static void sv_async_cb(uv_async_t* handle) {
  ThreadWorker* worker = (ThreadWorker*)handle->data;
  worker->_OnAsync(handle);
}

static void work_thread(void *arg)
{
#ifndef WIN32
  sigset_t signal_mask;
  sigemptyset(&signal_mask);
  sigaddset(&signal_mask, SIGPIPE);
  int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
  if (rc != 0)
  {
    ORPC_LOG(ERROR) << "pthread_sigmask error!";
    return;
  }
#endif
  ThreadWorker* worker = (ThreadWorker*)arg;
  worker->_DoWork();
  worker->_AfterWork();
}

int ThreadWorker::_AfterWork()
{
  m_threadId = (uv_thread_t)-1;
  return 0;
}

int ThreadWorker::Start()
{
  int r;

  r = uv_thread_create(&m_threadId, work_thread, (void*)this);
  UV_CHECK_RET_1(uv_thread_create, r);

  return 0;
}

int ThreadWorker::Stop()
{
  if (m_threadId == (uv_thread_t)-1)
    return 0;

  int r;

  r = uv_async_send(&m_asyncHandle);
  UV_CHECK_RET_1(uv_async_send, r);

  return 0;
}

int ThreadWorker::Join()
{
  if (m_threadId == (uv_thread_t)-1)
    return 0;

  int r;

  r = uv_thread_join(&m_threadId);
  UV_CHECK_RET_1(uv_thread_join, r);

  m_threadId = (uv_thread_t)-1;

  return 0;
}

bool ThreadWorker::IsThreadWorking() const
{
  return m_threadId != (uv_thread_t)-1;
}

int ThreadWorker::_PrepareWork()
{
  int r;

  r = uv_loop_init(&m_loop);
  UV_CHECK_RET_1(uv_loop_init, r);

  m_asyncHandle.data = this;
  r = uv_async_init(&m_loop, &m_asyncHandle, sv_async_cb);
  UV_CHECK_RET_1(uv_async_init, r);
  uv_unref((uv_handle_t*)&m_asyncHandle);

  return 0;
}

static void close_cb(uv_handle_t* handle)
{
  ThreadWorker* worker = (ThreadWorker*)handle->data;
  worker->_OnDestory(handle);
}

int ThreadWorker::_OnAsync(uv_async_t* handle)
{
  assert(handle == &m_asyncHandle);

  uv_close((uv_handle_t*)&m_asyncHandle, close_cb);
  return 0;
}

_END_ORPC_NAMESPACE_
