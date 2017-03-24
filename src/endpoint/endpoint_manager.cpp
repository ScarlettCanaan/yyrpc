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

#include "endpoint_manager.h"
#include "endpoint.h"
#include "../../proto/common_api.h"

_START_ORPC_NAMESPACE_

EndPointManager::EndPointManager() 
{

}

EndPointManager::~EndPointManager()
{
  m_apiToEndPoint.clear();
  for (auto it : m_endPoints)
    it->UnInit();
  m_endPoints.clear();
}

EndPointWrapper EndPointManager::QueryEndPoint(const std::string& api) const
{
  static EndPointWrapper null;

  std::lock_guard<std::mutex> l(m_endpointMutex);
  auto it = m_apiToEndPoint.find(api);
  if (it == m_apiToEndPoint.end())
    return null;

  return it->second;
}

EndPointWrapper EndPointManager::CreateEndPoint(const std::string& ip, int32_t port, int32_t flags)
{
#ifndef ORPC_SUPPROT_HTTP
  if (IS_HTTP(flags))
    throw std::runtime_error("not support http.");
#endif  //! #ifdef ORPC_SUPPROT_HTTP

  static EndPointWrapper null;

  std::lock_guard<std::mutex> l(m_endpointMutex);
  if (IS_TCP(flags))
    return CreateTcpEndPoint(ip, port, flags & METHOD_PROTOCAL_MASK);

  return null;
}

bool EndPointManager::RegisterApi(int32_t appId, int32_t privilege, const ServiceItem& item)
{
  int32_t flags = item.transport_protocol << 8 | item.method_protocol;

  EndPointWrapper wrapper = CreateEndPoint(item.ip, item.port, flags);

  wrapper.endpoint->SetAuth(appId, privilege, item.token);

  std::lock_guard<std::mutex> l(m_endpointMutex);
  m_apiToEndPoint[item.method_name] = wrapper.endpoint;
  return true;
}

EndPointWrapper EndPointManager::CreateTcpEndPoint(const std::string& ip, int32_t port, int32_t flags)
{
  bool bForceCreate = IS_FORCE_CREATE(flags) ? true : false;
  auto it = m_endPoints.begin();
  for (; !bForceCreate && it != m_endPoints.end(); ++it)
  {
    if ((*it)->GetIp() == ip && (*it)->GetPort() == port && (*it)->GetTransportProtocal() == TP_TCP && (*it)->GetTransportFlags() == flags)
      return *it;
  }

  std::shared_ptr<EndPoint> p(new TcpEndPoint(ip, port, flags));
  p->Init();
  m_endPoints.push_back(p);
  return p;
}

bool EndPointManager::CheckEndpointAllReady(uint32_t timeoutMs)
{
  uint32_t timeMs = 0;
  for (; timeMs < timeoutMs; timeMs += 100)
  {
    if (IsAllEndpointReady())
      return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return false;
}

bool EndPointManager::IsAllEndpointReady()
{
  auto it = m_endPoints.begin();
  for (; it != m_endPoints.end(); ++it)
  {
    if ((*it)->GetConnectStatus() != CS_CONNECTED && (*it)->ResetFail())
      return false;
  }

  return true;
}

_END_ORPC_NAMESPACE_
