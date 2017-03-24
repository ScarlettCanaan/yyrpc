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

#ifndef ORPC_LISTENPOINT_MANAGER_H_
#define ORPC_LISTENPOINT_MANAGER_H_

#include "../build_config.h"
#include <memory>
#include <list>
#include <unordered_map>
#include "../transport/transport.h"

_START_ORPC_NAMESPACE_

class ListenPoint;

struct ListenPointWrapper
{
public:
  ListenPointWrapper(){}
  ListenPointWrapper(const std::shared_ptr<ListenPoint>& ep) : endpoint(ep) {}
public:
  operator bool() const { return endpoint.get() != 0; }
  std::shared_ptr<ListenPoint> endpoint;
};

class RpcTcpServerTransport;

class ListenPointManager
{
public:
  ListenPointWrapper CreateListenPoint(const std::string& ip, int32_t port, int32_t flags);
private:
  ListenPointWrapper CreateTcpListenPoint(const std::string& ip, int32_t port, int32_t flags);
private:
  std::list<std::shared_ptr<ListenPoint>> m_listenPoints;
  mutable std::mutex m_endpointMutex;
private:
  friend class CalleeManager;
  ListenPointManager();
  ~ListenPointManager();
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_LISTENPOINT_MANAGER_H_
