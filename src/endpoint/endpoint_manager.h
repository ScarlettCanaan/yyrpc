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

#ifndef ORPC_ENDPOINT_MANAGER_H_
#define ORPC_ENDPOINT_MANAGER_H_

#include "../build_config.h"
#include <memory>
#include <list>
#include <mutex>
#include <unordered_map>
#include "../transport/transport.h"
#include "endpoint.h"

struct ServiceItem;

_START_ORPC_NAMESPACE_

struct EndPointWrapper 
{
public:
  EndPointWrapper() {}
  EndPointWrapper(const std::shared_ptr<EndPoint>& ep) : endpoint(ep) {}
public:
  operator bool() const { return endpoint.get() != 0 && !endpoint->IsFinalFailed(); }
public:
  std::shared_ptr<EndPoint> endpoint;
};

class EndPointManager 
{
public:
  EndPointWrapper QueryEndPoint(const std::string& api) const;
  EndPointWrapper CreateEndPoint(const std::string& ip, int32_t port, int32_t flags);
  bool RegisterApi(int32_t appId, int32_t privilege, const ServiceItem& item);

  bool CheckEndpointAllReady(uint32_t timeoutMs = -1);
private:
  bool IsAllEndpointReady();
  EndPointWrapper CreateTcpEndPoint(const std::string& ip, int32_t port, int32_t flags);
private:
  std::list<std::shared_ptr<EndPoint>> m_endPoints;
  std::unordered_map<std::string, std::shared_ptr<EndPoint>> m_apiToEndPoint;
  mutable std::mutex m_endpointMutex;
private:
  friend class CallerManager;
  EndPointManager();
  ~EndPointManager();
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_ENDPOINT_MANAGER_H_
