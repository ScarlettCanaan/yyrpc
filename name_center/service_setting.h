#ifndef ORPC_SERVICE_SETTING_H_
#define ORPC_SERVICE_SETTING_H_

#include <string>
#include <unordered_map>
#include "transport/transport.h"
#include "ini/ini.hpp"
#include <memory>
#include "../proto/common_api.h"

using namespace orpc;

class orpc::RpcTcpServerTransport;

enum ServiceType
{
  ST_Client,
  ST_Server,
};

struct ApiProvideSetting
{
  ApiProvideSetting(const std::string& val);
  bool isValid; //input may be invalid
  std::string apiName;
  bool nostate;
  int32_t privilege;
};

struct ApiDepencySetting
{
  ApiDepencySetting(const std::string& val);
  bool isValid;  //input may be invalid
  std::string apiName;
  bool nostate;
};

struct ServiceSetting
{
  std::string name;
  ServiceType type;
  int32_t appId;
  std::string  appToken;
  int32_t pingInterval;
  int32_t tProtocal;
  int32_t mProtocal;
  std::list<ApiDepencySetting> depency_apis;
  int32_t privilege;

  //only for server
  std::string listenIp;
  int32_t listenPort;
  std::list<ApiProvideSetting> provide_apis;

  void InitServiceItem(ServiceItem& item);
};

class ServiceSettingManager
{
public:
  static ServiceSettingManager& GetInstance();
  int Init(const std::string& cfgFile);
  int UnInit();

  std::shared_ptr<ServiceSetting>& GetServiceSettings(int32_t appId);
  std::shared_ptr<ServiceSetting>& GetServiceSettings(const std::string& methodName);
  const std::set<int32_t>& GetDepencyIds(int32_t appId);
  const std::set<int32_t>& GetProvideIds(int32_t appId);
private:
  int RegisterService(const std::string& serviceName, INI::Level& level);

  int RegisterDepencyApi(std::shared_ptr<ServiceSetting>& setting, INI::Level& level);
  int RegisterProvideApi(std::shared_ptr<ServiceSetting>& setting, INI::Level& level);
  int RegisterProvideApi(std::shared_ptr<ServiceSetting>& setting, INI::Level& level, int i);
  int RegisterDepencyApi(std::shared_ptr<ServiceSetting>& setting, INI::Level& level, int i);

  int UpdateAllRelation();
  int UpdateRelation(std::shared_ptr<ServiceSetting>& setting);
  int UpdateRelationWithDepencyApi(std::shared_ptr<ServiceSetting>& setting, ApiDepencySetting& api);
private:
  ServiceSettingManager();
  ~ServiceSettingManager();
private:
  //key:appId, value:setting
  std::unordered_map<int32_t, std::shared_ptr<ServiceSetting>> m_id2settings;
  //key:apiName, value:setting
  std::unordered_map<std::string, std::shared_ptr<ServiceSetting>> m_api2settings;
  // 1 client : n server, see UpdateAllRelation
  std::unordered_map<int32_t, std::set<int32_t>> m_depencys;
  // 1 server : n client, see UpdateAllRelation
  std::unordered_map<int32_t, std::set<int32_t>> m_provides;
};

#endif //! #ifndef ORPC_SERVICE_SETTING_H_
