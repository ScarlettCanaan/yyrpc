#include "endpoint_manager.h"
#include "endpoint.h"

EndPointManager& EndPointManager::GetInstance()
{
  static EndPointManager inst;
  return inst;
}

int EndPointManager::Init()
{
  return 0;
}

int EndPointManager::UnInit()
{
  return 0;
}

EndPointManager::EndPointManager()
{

}

EndPointManager::~EndPointManager()
{

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

EndPointWrapper EndPointManager::CreateEndPoint(const std::string& ip, int32_t port, TransportProtocol tProtocal, MethodProtocol mProtocal)
{
  static EndPointWrapper null;

  std::lock_guard<std::mutex> l(m_endpointMutex);
  if (tProtocal == TP_TCP)
    return CreateTcpEndPoint(ip, port, mProtocal);

  return null;
}

bool EndPointManager::RegisterApi(const std::string& api, const std::shared_ptr<EndPoint>& endpoint)
{
  std::lock_guard<std::mutex> l(m_endpointMutex);
  m_apiToEndPoint[api] = endpoint;
  return true;
}

EndPointWrapper EndPointManager::CreateTcpEndPoint(const std::string& ip, int32_t port, MethodProtocol mProtocal)
{
  auto it = m_endPoints.begin();
  for (; it != m_endPoints.end(); ++it)
  {
    if ((*it)->GetIp() == ip && (*it)->GetPort() == port && (*it)->GetTransportProtocal() == TP_TCP && (*it)->GetMethodProtocal() == mProtocal)
      return *it;
  }

  std::shared_ptr<EndPoint> p(new TcpEndPoint(ip, port, mProtocal));
  p->Init();
  m_endPoints.push_back(p);
  return p;
}
