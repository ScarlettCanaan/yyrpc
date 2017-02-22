#include "endpoint_manager.h"

EndPointManager& EndPointManager::GetInstance()
{
  static EndPointManager inst;
  return inst;
}

EndPointManager::EndPointManager()
{

}

EndPointManager::~EndPointManager()
{

}

const std::shared_ptr<EndPoint>& EndPointManager::QueryEndPoint(const std::string& api) const
{
  static std::shared_ptr<EndPoint> null;
  return null;
}

const std::shared_ptr<EndPoint>& EndPointManager::CreateEndPoint(const std::string& ip, int32_t port, TransportProtocol protocal, EndPointFlag flag/* = EPF_DEFAULT*/)
{
  static std::shared_ptr<EndPoint> null;
  return null;
}

bool EndPointManager::RegisterApi(const std::string& api, const std::shared_ptr<EndPoint>& endpoint)
{
  return true;
}
