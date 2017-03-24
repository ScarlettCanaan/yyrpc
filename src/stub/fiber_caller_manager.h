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

#ifndef ORPC_FIBER_CALL_MANAGER_H_
#define ORPC_FIBER_CALL_MANAGER_H_

#include "../build_config.h"

#ifdef ORPC_USE_FIBER

#include <map>
#include <memory>
#include <list>
#include <thread>
#include <mutex>
#include <functional>
#include <unordered_map>
#include "../build_config.h"
#include "call_result.h"

_START_ORPC_NAMESPACE_

struct FunctionWrapper
{
  std::function<int(void)> func;
};

class FiberCallerManager
{
public:
  FiberCallerManager(size_t maxFiberNum);
  ~FiberCallerManager();
public:
  bool OnResult(std::thread::id id, std::shared_ptr<ICallResult> result);
  bool PumpMessage();

  template<typename R>
  CallResultWrapper<R> NOTHROW TryBlock(CallResultWrapper<R>& wrapper);

  int NOTHROW CreateFiber(const std::function<int(void)>& f);
  void NOTHROW _OnFiberWork();
private:
  std::thread::id m_initThreadId;
  size_t m_maxFiberNum;

  mutable std::mutex m_resultMutex;
  std::map<std::thread::id, std::list<std::shared_ptr<ICallResult>>> m_specThreadResult;
  std::unordered_map<st_thread_t, std::shared_ptr<FunctionWrapper>> m_fiberThreadMap;
};

template<typename R>
CallResultWrapper<R> FiberCallerManager::TryBlock(CallResultWrapper<R>& wrapper)
{
  if (!wrapper)
    return wrapper;

  st_cond_t t = wrapper.m_result->GetFiberCond();
  wrapper.m_result->SetFiberCallback([t]() { st_cond_signal(t);});

  return wrapper;
}

_END_ORPC_NAMESPACE_

#endif  //! #ifdef ORPC_USE_FIBER

#endif  //! #ifndef ORPC_FIBER_CALL_MANAGER_H_
