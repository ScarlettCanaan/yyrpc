#ifndef YYRPC_LISTENPOINT_MANAGER_H_
#define YYRPC_LISTENPOINT_MANAGER_H_

#include <memory>
#include "listenpoint.h"
#include <list>
#include <unordered_map>

class ListenPointManager
{
public:
  static ListenPointManager& GetInstance();
public:
  std::shared_ptr<ListenPoint> Create(const std::string& ip, int32_t port, TransportProtocol protocal);
  bool RegisterApi(const std::string& api, const std::shared_ptr<ListenPoint>& listenpoint);
private:
  std::shared_ptr<ListenPoint> CreateTcpListenPoint(const std::string& ip, int32_t port);
private:
  ListenPointManager();
  ~ListenPointManager();
private:
  std::list<std::shared_ptr<ListenPoint>> m_listenPoints;
  std::unordered_map<std::string, std::shared_ptr<ListenPoint>> m_apiToListenPoint;
};

#endif  //! #ifndef YYRPC_LISTENPOINT_MANAGER_H_
