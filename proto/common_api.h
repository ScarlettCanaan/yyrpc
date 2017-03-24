#ifndef ORPC_COMMON_API_H
#define ORPC_COMMON_API_H

#include "../src/event.h"
#include "../src/method.h"
#include "../src/property.h"
#include "../src/transport/transport.h"
#include <string>
#include <list>

struct ServiceItem
{
  std::string method_name;
  std::string ip;
  int32_t port;
  int32_t transport_protocol;
  int32_t method_protocol;
  int64_t token;
  ORPC_PROPERTY(method_name, ip, port, transport_protocol, method_protocol, token);
};

struct MethodItem
{
  std::string method_name;
  bool nostate;
  int32_t privilege;
  ORPC_PROPERTY(method_name, nostate, privilege);
};

struct HelloClientResponse
{
  int32_t privilege;
  std::list<ServiceItem> services;
  ORPC_PROPERTY(privilege, services);
};

struct ClientInfo
{
  int32_t appId;
  int64_t token;
  int32_t privilege;
  ORPC_PROPERTY(appId, token, privilege);
};

struct HelloServerResponse
{
  HelloClientResponse clientResponse;
  std::list<ClientInfo> clientInfos;
  std::list<MethodItem> methods;
  ORPC_PROPERTY(clientResponse, clientInfos, methods);
};

namespace NameCenterAPi
{
  ORPC_METHOD(SubscribeEvent, bool(const std::string& eventName));
  ORPC_METHOD(HelloClientService, HelloClientResponse(int32_t appId, const std::string& token));
  ORPC_METHOD(HelloServerService, HelloServerResponse(int32_t appId, const std::string& token));

  ORPC_EVENT(OnClientConnect, void(int32_t appId, int64_t token, int32_t privilege));
  ORPC_EVENT(OnClientDisconnected, void(int32_t appId));
}

#endif  //! #ifndef TEST_API_H
