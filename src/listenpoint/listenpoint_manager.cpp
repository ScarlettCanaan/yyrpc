#include "listenpoint_manager.h"
#include "listenpoint.h"

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

ListenPointWrapper ListenPointManager::CreateListenPoint(const std::string& ip, int32_t port, TransportProtocol protocal)
{
  static std::shared_ptr<ListenPoint> null;

  std::lock_guard<std::mutex> l(m_endpointMutex);
  if (protocal == TP_TCP)
    return CreateTcpListenPoint(ip, port);

  return null;
}

ListenPointWrapper ListenPointManager::CreateTcpListenPoint(const std::string& ip, int32_t port)
{
  auto it = m_listenPoints.begin();
  for (; it != m_listenPoints.end(); ++it)
  {
    if ((*it)->GetIp() == ip && (*it)->GetPort() == port && (*it)->GetProtocal() == TP_TCP)
      return *it;
  }

  std::shared_ptr<ListenPoint> p(new TcpListenPoint(ip, port));
  p->Init();
  m_listenPoints.push_back(p);
  return p;
}
