#ifndef YYRPC_ENDPOINT_MANAGER_H_
#define YYRPC_ENDPOINT_MANAGER_H_

#include <memory>
#include <list>
#include <unordered_map>
#include "transport/transport.h"
#include "endpoint.h"

class EndPoint;

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
  static EndPointManager& GetInstance();
public:
  int Init();
  int UnInit();
public:
  EndPointWrapper QueryEndPoint(const std::string& api) const;
  EndPointWrapper CreateEndPoint(const std::string& ip, int32_t port, TransportProtocol tProtocal, MethodProtocol mProtocal);
  bool RegisterApi(const std::string& api, const std::shared_ptr<EndPoint>& endpoint);
private:
  EndPointWrapper CreateTcpEndPoint(const std::string& ip, int32_t port, MethodProtocol mProtocal);
private:
  EndPointManager();
  ~EndPointManager();
private:
  std::list<std::shared_ptr<EndPoint>> m_endPoints;
  std::unordered_map<std::string, std::shared_ptr<EndPoint>> m_apiToEndPoint;
  mutable std::mutex m_endpointMutex;
};

#endif  //! #ifndef YYRPC_ENDPOINT_MANAGER_H_
