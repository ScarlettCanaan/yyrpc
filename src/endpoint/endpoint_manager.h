#ifndef YYRPC_ENDPOINT_MANAGER_H_
#define YYRPC_ENDPOINT_MANAGER_H_

#include <memory>
#include "endpoint.h"
#include <list>
#include <unordered_map>

class EndPointManager
{
public:
  static EndPointManager& GetInstance();
public:
  const std::shared_ptr<EndPoint>& QueryEndPoint(const std::string& api) const;
  const std::shared_ptr<EndPoint>& CreateEndPoint(const std::string& ip, int32_t port, TransportProtocol protocal, EndPointFlag flag = EPF_DEFAULT);
  bool RegisterApi(const std::string& api, const std::shared_ptr<EndPoint>& endpoint);
private:
  EndPointManager();
  ~EndPointManager();
private:
  std::list<std::shared_ptr<EndPoint>> m_endPoints;
  std::unordered_map<std::string, std::shared_ptr<EndPoint>> m_apiToEndPoint;
};

#endif  //! #ifndef YYRPC_ENDPOINT_MANAGER_H_
