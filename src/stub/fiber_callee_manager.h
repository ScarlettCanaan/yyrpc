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

#ifndef ORPC_FIBER_CALLEE_MANAGER_H_
#define ORPC_FIBER_CALLEE_MANAGER_H_

#include "../build_config.h"

#ifdef ORPC_USE_FIBER

#include <string>
#include <memory>
#include <unordered_map>
#include <list>
#include <thread>
#include <mutex>
#include "callback.h"
#include "../build_config.h"
#include "../util/thread_safe_list.h"

_START_ORPC_NAMESPACE_

class RpcTcpServerTransport;
class ICallback;
struct CalleePacket;

class FiberCalleeManager
{
public:
  FiberCalleeManager(size_t maxFiberNum);
  ~FiberCalleeManager();
public:
  int OnCall(const std::shared_ptr<CalleePacket>& packet);

  bool NOTHROW PumpMessage();
  void NOTHROW _OnFiberWork();
private:
  int NOTHROW DoTask(const std::shared_ptr<CalleePacket>& task);
private:
  mutable std::mutex m_caleeMutex;
  std::map<std::thread::id, std::list<std::shared_ptr<CalleePacket>>> m_specThreadCallee;

  std::thread::id m_initThreadId;
  size_t m_maxFiberNum;

  std::unordered_map<st_thread_t, std::shared_ptr<CalleePacket>> m_fiberThreadMap;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifdef ORPC_USE_FIBER

#endif  //! #ifndef ORPC_FIBER_CALLEE_MANAGER_H_
