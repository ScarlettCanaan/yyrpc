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

#ifndef ORPC_SERVER_WORKER_H_
#define ORPC_SERVER_WORKER_H_

#include "../build_config.h"

#ifndef ORPC_USE_FIBER

#include "../thread/thread_worker.h"
#include <memory>
#include "../util/thread_safe_list.h"

_START_ORPC_NAMESPACE_

struct CalleePacket;

class ServerWorker : public ThreadWorker
{
public:
  ServerWorker();
  ~ServerWorker();
public:
  // ThreadWorker
  int _DoWork() override;
  int _OnAsync(uv_async_t* handle) override;
  int _OnDestory(uv_handle_t* handle) override;

  int _OnIdle(uv_idle_t *handle);
public:
  int QueueTask(const std::shared_ptr<CalleePacket>& task);
private:
  int DoTask(const std::shared_ptr<CalleePacket>& task);
private:
  uv_idle_t m_idler;
  ThreadSafeList<std::shared_ptr<CalleePacket>> m_taskList;
};

_END_ORPC_NAMESPACE_

#endif //! #ifndef ORPC_USE_FIBER

#endif  //! #ifndef ORPC_SERVER_WORKER_H_
