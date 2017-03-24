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

#include "client_worker_pool.h"

#ifndef ORPC_USE_FIBER

#include "client_worker.h"

_START_ORPC_NAMESPACE_

ClientWorkerPool::ClientWorkerPool(size_t worker_num)
{
  m_workers.reserve(worker_num);
  for (size_t i = 0; i < worker_num; ++i)
    m_workers.push_back(std::make_shared<ClientWorker>());
}

ClientWorkerPool::~ClientWorkerPool()
{
  m_workers.clear();
}

int ClientWorkerPool::QueuePacket(size_t groupId, const std::shared_ptr<ICallResult>& task)
{
  if (m_workers.empty())
  {
    ORPC_LOG(ERROR) << "no worker in pool.";
    return -1;
  }

  size_t index = groupId % m_workers.size();
  return m_workers[index]->QueueTask(task);
}

_END_ORPC_NAMESPACE_

#endif //! #ifndef ORPC_USE_FIBER

