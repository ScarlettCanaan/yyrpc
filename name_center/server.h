#ifndef ORPC_SERVER_H_
#define ORPC_SERVER_H_

#include "rpc_application.h"
#include "../proto/common_api.h"

using namespace orpc;

class NameServerApp : public RpcNameCenterApplication, public ConnectionMonitor
{
public:
  NameServerApp(const char* cfgFile);
  ~NameServerApp();
private:
  int Init(const char* cfgFile);
  int UnInit();
private:
  bool BindService();
  HelloClientResponse OnClientHello(orpc::TcpPeer* peer, int32_t appId, const std::string& token);
  HelloServerResponse OnServerHello(orpc::TcpPeer* peer, int32_t appId, const std::string& token);
  bool OnSubscribeEvent(orpc::TcpPeer* peer, const std::string& eventName);
private:
  bool OnConnected(orpc::TcpPeer* peer) override;
  bool OnDisConnected(orpc::TcpPeer* peer) override;
};

#endif //! #ifndef ORPC_SERVER_H_
