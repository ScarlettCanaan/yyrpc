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

#include "listenpoint_manager.h"
#include "listenpoint.h"
#include "../transport/rpc_tcp_server_transport.h"

_START_ORPC_NAMESPACE_

ListenPointManager::ListenPointManager()
{

}

ListenPointManager::~ListenPointManager()
{
  for (auto it : m_listenPoints)
    it->UnInit();
  m_listenPoints.clear();
}

ListenPointWrapper ListenPointManager::CreateListenPoint(const std::string& ip, int32_t port, int32_t flags)
{
#ifndef ORPC_SUPPROT_HTTP
  if (IS_HTTP(flags))
    throw std::runtime_error("not support http.");
#endif  //! #ifdef ORPC_SUPPROT_HTTP

  static std::shared_ptr<ListenPoint> null;

  std::lock_guard<std::mutex> l(m_endpointMutex);
  if (IS_TCP(flags))
    return CreateTcpListenPoint(ip, port, flags & METHOD_PROTOCAL_MASK);

  return null;
}

ListenPointWrapper ListenPointManager::CreateTcpListenPoint(const std::string& ip, int32_t port, int32_t flags)
{
  auto it = m_listenPoints.begin();
  for (; it != m_listenPoints.end(); ++it)
  {
    if ((*it)->GetIp() == ip && (*it)->GetPort() == port && (*it)->GetProtocal() == TP_TCP && (*it)->GetTransportFlags() == flags)
      return *it;
  }

  std::shared_ptr<ListenPoint> p(new TcpListenPoint(ip, port, flags));
  p->Init();
  m_listenPoints.push_back(p);
  return p;
}

_END_ORPC_NAMESPACE_
