#include <iostream>
#include <string>
#include "service_manager.h"
#include "service_setting.h"
#include "listenpoint/listenpoint_manager.h"
#include <functional>
#include "../proto/common_api.h"

ServiceManager& ServiceManager::GetInstance()
{
  static ServiceManager inst;
  return inst;
}

int ServiceManager::Init()
{
  return 0;
}

int ServiceManager::UnInit()
{
  return 0;
}

void ServiceManager::FireOnClientConnect(int32_t clientId, int32_t depencyId)
{
  auto peerIt = m_connectedIdServices.find(depencyId);
  if (peerIt == m_connectedIdServices.end())
    return;

  int32_t privilege = m_privilegeCache[clientId];
  int64_t key = (int64_t)((uint32_t)clientId << 31) | depencyId;
  int64_t session_token = m_sessionTokens[key];
  BufferStream data = NameCenterAPi::OnClientConnect.GetFireData(clientId, session_token, privilege);
  orpc::TcpPeer* peer = peerIt->second;
  peer->SendData(data);
}

void ServiceManager::FireOnClientDisConnect(int32_t clientId, int32_t depencyId)
{
  auto peerIt = m_connectedIdServices.find(depencyId);
  if (peerIt == m_connectedIdServices.end())
    return;

  BufferStream data = NameCenterAPi::OnClientDisconnected.GetFireData(clientId);
  orpc::TcpPeer* peer = peerIt->second;
  peer->SendData(data);
}

void ServiceManager::FireOnClientConnect(int32_t clientId)
{
  const std::set<int32_t> depencys = ServiceSettingManager::GetInstance().GetDepencyIds(clientId);
  for (auto it : depencys)
    FireOnClientConnect(clientId, it);
}

void ServiceManager::FireOnClientDisConnect(int32_t clientId)
{
  const std::set<int32_t> depencys = ServiceSettingManager::GetInstance().GetDepencyIds(clientId);
  for (auto it : depencys)
    FireOnClientDisConnect(clientId, it);
}

HelloClientResponse ServiceManager::OnClientHello(orpc::TcpPeer* peer, int32_t appId, const std::string& token)
{
  HelloClientResponse response;
  response.privilege = -1;
  auto setting = ServiceSettingManager::GetInstance().GetServiceSettings(appId);
  if (!setting)
  {
    peer->CloseConnection();
    return response;
  }

  if (setting->appToken != token)
  {
    peer->CloseConnection();
    return response;
  }

  response.privilege = setting->privilege;

  m_connectedPeerServices[peer] = appId;
  m_connectedIdServices[appId] = peer;
  m_privilegeCache[appId] = response.privilege;
  const std::set<int32_t> depencys = ServiceSettingManager::GetInstance().GetDepencyIds(appId);
  for (auto it : depencys)
  {
    int64_t key = (int64_t)((uint32_t)appId << 31) | it;
    size_t token_hash = std::hash<std::string>()(token);
    m_sessionTokens[key] = (int64_t)peer + (int64_t)token_hash;
    m_sessionKeys[appId].push_back(key);
  }

  for (auto it : setting->depency_apis)
  {
    auto setting_depency = ServiceSettingManager::GetInstance().GetServiceSettings(it.apiName);
    if (!setting_depency)
      continue;
    ServiceItem item;
    item.method_name = it.apiName;
    setting_depency->InitServiceItem(item);
    int64_t key = (int64_t)((uint32_t)appId << 31) | setting_depency->appId;
    item.token = m_sessionTokens[key];
    response.services.push_back(item);
  }

  FireOnClientConnect(appId);

  return response;
}

HelloServerResponse ServiceManager::OnServerHello(orpc::TcpPeer* peer, int32_t appId, const std::string& token)
{
  HelloServerResponse serverRsp;
  serverRsp.clientResponse = OnClientHello(peer, appId, token);
  if (serverRsp.clientResponse.privilege == -1)
    return serverRsp;

  const std::set<int32_t> provides = ServiceSettingManager::GetInstance().GetProvideIds(appId);
  for (auto it : provides)
  {
    FillClientInfo(serverRsp, appId, it);
  }

  auto setting = ServiceSettingManager::GetInstance().GetServiceSettings(appId);
  for (auto it : setting->provide_apis)
  {
    MethodItem method;
    method.method_name = it.apiName;
    method.nostate = it.nostate;
    method.privilege = it.privilege;
    serverRsp.methods.push_back(method);
  }

  return serverRsp;
}

void ServiceManager::FillClientInfo(HelloServerResponse& serverRsp, int32_t appId, int32_t depencyId)
{
  auto peer = m_connectedIdServices.find(depencyId);
  if (peer == m_connectedIdServices.end())
    return;

  ClientInfo info;
  info.appId = depencyId;
  info.privilege = m_privilegeCache[depencyId];
  int64_t key = (int64_t)((uint32_t)depencyId << 31) | appId;
  info.token = m_sessionTokens[key];
  serverRsp.clientInfos.push_back(info);
}

void ServiceManager::OnOpenConnection(orpc::TcpPeer* peer)
{
  
}

void ServiceManager::OnCloseConnection(orpc::TcpPeer* peer)
{
  auto it = m_connectedPeerServices.find(peer);
  if (it == m_connectedPeerServices.end())
    return;
  FireOnClientDisConnect(it->second);

  m_connectedIdServices.erase(it->second);
  m_privilegeCache.erase(it->second);
  std::list<int64_t>& keys = m_sessionKeys[it->second];
  for (auto key : keys)
    m_sessionTokens.erase(key);
  m_sessionKeys.erase(it->second);

  m_connectedPeerServices.erase(it);
}

ServiceManager::ServiceManager()
{

}

ServiceManager::~ServiceManager()
{

}
