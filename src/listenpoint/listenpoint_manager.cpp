#include "listenpoint_manager.h"
#include "listenpoint.h"

ListenPointManager& ListenPointManager::GetInstance()
{
  static ListenPointManager inst;
  return inst;
}

int ListenPointManager::Init()
{
  return 0;
}

int ListenPointManager::UnInit()
{
  return 0;
}

ListenPointManager::ListenPointManager()
{

}

ListenPointManager::~ListenPointManager()
{

}

ListenPointWrapper ListenPointManager::CreateListenPoint(const std::string& ip, int32_t port, TransportProtocol protocal, MethodProtocol mProtocal)
{
  static std::shared_ptr<ListenPoint> null;

  std::lock_guard<std::mutex> l(m_endpointMutex);
  if (protocal == TP_TCP)
    return CreateTcpListenPoint(ip, port, mProtocal);

  return null;
}

ListenPointWrapper ListenPointManager::CreateTcpListenPoint(const std::string& ip, int32_t port, MethodProtocol mProtocal)
{
  auto it = m_listenPoints.begin();
  for (; it != m_listenPoints.end(); ++it)
  {
    if ((*it)->GetIp() == ip && (*it)->GetPort() == port && (*it)->GetProtocal() == TP_TCP && (*it)->GetMethodProtocal() == mProtocal)
      return *it;
  }

  std::shared_ptr<ListenPoint> p(new TcpListenPoint(ip, port, mProtocal));
  p->Init();
  m_listenPoints.push_back(p);
  return p;
}
