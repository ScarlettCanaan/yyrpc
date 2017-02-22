#include "listenpoint_manager.h"

ListenPointManager& ListenPointManager::GetInstance()
{
  static ListenPointManager inst;
  return inst;
}

ListenPointManager::ListenPointManager()
{

}

ListenPointManager::~ListenPointManager()
{

}

std::shared_ptr<ListenPoint> ListenPointManager::Create(const std::string& ip, int32_t port, TransportProtocol protocal)
{
  static std::shared_ptr<ListenPoint> null;

  if (protocal == TP_TCP)
    return CreateTcpListenPoint(ip, port);

  return null;
}

bool ListenPointManager::RegisterApi(const std::string& api, const std::shared_ptr<ListenPoint>& listenpoint)
{
  if (m_apiToListenPoint.find(api) != m_apiToListenPoint.end())
    return false;

  m_apiToListenPoint[api] = listenpoint;
  return true;
}

std::shared_ptr<ListenPoint> ListenPointManager::CreateTcpListenPoint(const std::string& ip, int32_t port)
{
  auto it = m_listenPoints.begin();
  for (; it != m_listenPoints.end(); ++it)
  {
    if ((*it)->GetIp() == ip && (*it)->GetPort() == port && (*it)->GetProtocal() == TP_TCP)
      return *it;
  }

  std::shared_ptr<ListenPoint> p(new ListenPoint(ip, port, TP_TCP));
  p->Init();
  m_listenPoints.push_back(p);
  return p;
}
