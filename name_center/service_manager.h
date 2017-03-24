#ifndef ORPC_SERVICE_MANAGER_H_
#define ORPC_SERVICE_MANAGER_H_

#include "../proto/common_api.h"

using namespace orpc;
class orpc::TcpPeer;

class ServiceManager
{
public:
  static ServiceManager& GetInstance();
  int Init();
  int UnInit();
public:
  HelloClientResponse OnClientHello(orpc::TcpPeer* peer, int32_t appId, const std::string& token);
  HelloServerResponse OnServerHello(orpc::TcpPeer* peer, int32_t appId, const std::string& token);
  void OnOpenConnection(orpc::TcpPeer* peer);
  void OnCloseConnection(orpc::TcpPeer* peer);
private:
  void FireOnClientConnect(int32_t clientId);
  void FireOnClientDisConnect(int32_t clientId);
  void FireOnClientConnect(int32_t clientId, int32_t depencyId);
  void FireOnClientDisConnect(int32_t clientId, int32_t depencyId);
  void FillClientInfo(HelloServerResponse& serverRsp, int32_t appId, int32_t depencyId);
private:
  ServiceManager();
  ~ServiceManager();
  std::unordered_map<orpc::TcpPeer*, int32_t> m_connectedPeerServices;
  std::unordered_map<int32_t, orpc::TcpPeer*> m_connectedIdServices;
  //key: (appId << 32 ) | depencyAppId, value: session
  std::unordered_map<int64_t, int64_t> m_sessionTokens;
  std::unordered_map<int32_t, int32_t> m_privilegeCache;
  std::unordered_map<int32_t, std::list<int64_t>> m_sessionKeys;
};

#endif //! #ifndef ORPC_SERVICE_MANAGER_H_
