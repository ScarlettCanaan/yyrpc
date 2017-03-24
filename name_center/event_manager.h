#ifndef ORPC_EVENT_MANAGER_H_
#define ORPC_EVENT_MANAGER_H_

#include <string>
#include <unordered_map>
#include "transport/transport.h"
#include "ini/ini.hpp"
#include <memory>
#include "../proto/common_api.h"
#include "stub/callee_manager.h"

using namespace orpc;

class orpc::TcpPeer;

class EventManager : public EventDispatcher
{
public:
  static EventManager& GetInstance();
  int Init(const std::string& cfgFile);
  int UnInit();

  bool Register(const std::string& eventName, orpc::TcpPeer* peer);
  bool UnRegister(orpc::TcpPeer* peer);

  bool OnEvent(const std::string& eventName, const std::string& eventData);
private:
  bool UnRegister(const std::string& eventName, orpc::TcpPeer* peer);
private:
  EventManager();
  ~EventManager();
private:
  std::map<std::string, std::list<orpc::TcpPeer*>> m_event2Peer;
  std::map<orpc::TcpPeer*, std::list<std::string>> m_peer2Event;
};

#endif //! #ifndef ORPC_EVENT_MANAGER_H_
