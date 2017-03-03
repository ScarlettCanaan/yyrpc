#ifndef YYRPC_LISTENPOINT_MANAGER_H_
#define YYRPC_LISTENPOINT_MANAGER_H_

#include <memory>
#include <list>
#include <unordered_map>
#include "transport/transport.h"

class ListenPoint;

struct ListenPointWrapper
{
public:
  ListenPointWrapper(){}
  ListenPointWrapper(const std::shared_ptr<ListenPoint>& ep) : endpoint(ep) {}
public:
  operator bool() const { return endpoint.get() != 0; }
  std::shared_ptr<ListenPoint> endpoint;
};

class ListenPointManager
{
public:
  static ListenPointManager& GetInstance();
public:
  int Init();
  int UnInit();
public:
  ListenPointWrapper CreateListenPoint(const std::string& ip, int32_t port, TransportProtocol protocal, MethodProtocol mProtocal);
private:
  ListenPointWrapper CreateTcpListenPoint(const std::string& ip, int32_t port, MethodProtocol mProtocal);
private:
  ListenPointManager();
  ~ListenPointManager();
private:
  std::list<std::shared_ptr<ListenPoint>> m_listenPoints;
  mutable std::mutex m_endpointMutex;
};

#endif  //! #ifndef YYRPC_LISTENPOINT_MANAGER_H_
