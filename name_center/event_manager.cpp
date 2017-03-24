#include <iostream>
#include <string>
#include "build_config.h"
#include "event_manager.h"
#include "ini/ini.hpp"
#include "listenpoint/listenpoint_manager.h"
#include "orpc.h"
#include "rpc_application.h"

EventManager& EventManager::GetInstance()
{
  static EventManager inst;
  return inst;
}

EventManager::EventManager()
{

}

EventManager::~EventManager()
{

}

int EventManager::Init(const std::string& cfgFile)
{
  RpcNameCenterApplication::SetEventDispatcher(this);
  return 0;
}

int EventManager::UnInit()
{
  return 0;
}

bool EventManager::Register(const std::string& eventName, orpc::TcpPeer* peer)
{
  m_event2Peer[eventName].push_back(peer);
  m_peer2Event[peer].push_back(eventName);
  return true;
}

bool EventManager::UnRegister(const std::string& eventName, orpc::TcpPeer* peer)
{
  auto itp = m_event2Peer.find(eventName);
  if (itp == m_event2Peer.end())
    return true;

  for (auto itnp = itp->second.begin(); itnp != itp->second.end();)
  {
    if (*itnp == peer)
      itnp = itp->second.erase(itnp);
    else
      ++itnp;
  }

  return true;
}

bool EventManager::UnRegister(orpc::TcpPeer* peer)
{
  auto it = m_peer2Event.find(peer);
  if (it == m_peer2Event.end())
    return true;

  for (auto event_name : it->second)
    UnRegister(event_name, peer);

  m_peer2Event.erase(it);
  return true;
}

bool EventManager::OnEvent(const std::string& eventName, const std::string& eventData)
{
  auto it = m_event2Peer.find(eventName);
  if (it == m_event2Peer.end())
    return false;

  std::list<orpc::TcpPeer*> listClone = it->second;
  for (auto peer : listClone)
  {
    BufferStream s;
    s.write(eventData.c_str(), eventData.length());
    peer->SendData(s);
  }
  
  return true;
}
