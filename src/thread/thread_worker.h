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

#ifndef _ORPC_THREAD_WORKER_H_
#define _ORPC_THREAD_WORKER_H_

#include "../build_config.h"
#include "uv.h"

_START_ORPC_NAMESPACE_

class ThreadWorker
{
public:
  ThreadWorker();
  virtual ~ThreadWorker();
public:
  int Start();
  int Stop();
  int Join();
  bool IsThreadWorking() const;
public:
  virtual int _PrepareWork();
  virtual int _DoWork() = 0;
  virtual int _OnDestory(uv_handle_t* handle) = 0;
  virtual int _OnAsync(uv_async_t* handle);
public:
  int _AfterWork();
protected:
  uv_loop_t m_loop;
  uv_thread_t m_threadId;
  uv_async_t m_asyncHandle;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef _ORPC_THREAD_WORKER_H_
