#include <iostream>
#include <string>
#include "../proto/test_api.h"
#include "server.h"
#include "orpc.h"
#include "service_setting.h"
#include "../proto/common_api.h"
#include "service_manager.h"
#include "listenpoint/listenpoint_manager.h"
#include "event_manager.h"
#include "glog/logging.h"

NameServerApp::NameServerApp(const char* cfgFile)
{
  bool bInited = RpcNameCenterApplication::Init(cfgFile);
  if (!bInited)
    throw std::runtime_error("failed to init rpc.");

  Init(cfgFile);
}

NameServerApp::~NameServerApp()
{
  UnInit();
}

int NameServerApp::Init(const char* cfgFile)
{
  ServiceSettingManager::GetInstance().Init("./service_config.cfg");
  EventManager::GetInstance().Init("./service_config.cfg");

  SetConnectMonitor(this);
  if (!BindService())
    return -1;

  return 0;
}

HelloClientResponse NameServerApp::OnClientHello(orpc::TcpPeer* peer, int32_t appId, const std::string& token)
{
  LOG(ERROR) << "client hello form appid:" << appId
    << " ip: " << peer->GetPeerIp() << " port: " << peer->GetPeerPort();
  return ServiceManager::GetInstance().OnClientHello(peer, appId, token);
}

HelloServerResponse NameServerApp::OnServerHello(orpc::TcpPeer* peer, int32_t appId, const std::string& token)
{
  LOG(ERROR) << "server hello form appid:" << appId
    << " ip: " << peer->GetPeerIp() << " port: " << peer->GetPeerPort();
  return ServiceManager::GetInstance().OnServerHello(peer, appId, token);
}

bool NameServerApp::OnSubscribeEvent(orpc::TcpPeer* peer, const std::string& eventName)
{
  LOG(ERROR) << "SubscribeEvent:" << eventName;
  EventManager::GetInstance().Register(eventName, peer);
  return true;
}

bool NameServerApp::BindService()
{
  NameCenterAPi::HelloClientService.BindWithPeer(
    std::bind(&NameServerApp::OnClientHello, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  NameCenterAPi::HelloServerService.BindWithPeer(
    std::bind(&NameServerApp::OnServerHello, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  NameCenterAPi::SubscribeEvent.BindWithPeer(std::bind(&NameServerApp::OnSubscribeEvent, this, std::placeholders::_1, std::placeholders::_2));

  return true;
}

int NameServerApp::UnInit()
{
  return 0;
}

bool NameServerApp::OnConnected(orpc::TcpPeer* peer)
{
  LOG(ERROR) << "NameServerApp::OnOpenConnection ip:" << peer->GetPeerIp() <<  " port:" << peer->GetPeerPort();
  ServiceManager::GetInstance().OnOpenConnection(peer);
  return true;
}

bool NameServerApp::OnDisConnected(orpc::TcpPeer* peer)
{
  LOG(ERROR) << "NameServerApp::OnCloseConnection ip:" << peer->GetPeerIp() << " port:" << peer->GetPeerPort();
  ServiceManager::GetInstance().OnCloseConnection(peer);
  EventManager::GetInstance().UnRegister(peer);
  return true;
}
