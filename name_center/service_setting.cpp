#include <iostream>
#include <string>
#include "build_config.h"
#include "service_setting.h"
#include "ini/ini.hpp"

void ServiceSetting::InitServiceItem(ServiceItem& item)
{
  item.method_protocol = mProtocal;
  item.transport_protocol = tProtocal;
  item.ip = listenIp;
  item.port = listenPort;
}

static std::string do_tokenize_method_name(const char* strPtr)
{
  while (*(strPtr - 1) == ' ' || *(strPtr - 1) == ']')
    strPtr--;
  const char* nameEnd = strPtr;
  while (*strPtr != ' ' && *strPtr != '=')
    strPtr--;
  const char* nameFirst = strPtr + 1;
  return std::string(nameFirst, nameEnd - nameFirst);
}

static std::vector<std::string> tokenize_method_name(const char* str)
{
  std::vector<std::string> tokens;
  const char* strPtr = str;

  while (*strPtr != 0)
  {
    if (*strPtr == ',')
      tokens.push_back(do_tokenize_method_name(strPtr));
    ++strPtr;
  }

  tokens.push_back(do_tokenize_method_name(strPtr));

  return tokens;
}

ApiProvideSetting::ApiProvideSetting(const std::string& val)
{
  std::vector<std::string> tokens = tokenize_method_name(val.c_str());
  isValid = (tokens.size() == 3);
  if (!isValid)
    return;

  apiName = tokens[0];
  nostate = tokens[1] == "true";
  privilege = StringToNumber<int32_t>(tokens[2]);
}

ApiDepencySetting::ApiDepencySetting(const std::string& val)
{
  std::vector<std::string> tokens = tokenize_method_name(val.c_str());
  isValid = (tokens.size() == 2);
  if (!isValid)
    return;

  apiName = tokens[0];
  nostate = tokens[1] == "true";
}

ServiceSettingManager& ServiceSettingManager::GetInstance()
{
  static ServiceSettingManager inst;
  return inst;
}

ServiceSettingManager::ServiceSettingManager()
{

}

ServiceSettingManager::~ServiceSettingManager()
{

}

int ServiceSettingManager::Init(const std::string& cfgFile)
{
  INI::Parser parse(cfgFile.c_str());
  INI::Level::section_map_t& sections = parse.top().sections;
  for (auto it = sections.begin(); it != sections.end(); ++it)
  {
    if (RegisterService(it->first, it->second) != 0)
      return -1;
  }

  if (UpdateAllRelation() != 0)
  { 
    LOG(ERROR) << "UpdateAllRelation failed.";
    return -1;
  }

  return 0;
}

int ServiceSettingManager::UnInit()
{
  return 0;
}

int ServiceSettingManager::RegisterService(const std::string& serviceName, INI::Level& level)
{
  std::shared_ptr<ServiceSetting> setting = std::make_shared<ServiceSetting>();
  setting->name = serviceName;

  std::string type = level["type"];
  setting->type = type == "server" ? ST_Server : ST_Client;
  setting->appId = StringToNumber<int32_t>(level["app_id"]);
  if (m_id2settings.find(setting->appId) != m_id2settings.end())
  {
    LOG(ERROR) << "serviceName is dup: " << serviceName << " id:" << setting->appId;
    return -1;
  }

  setting->appToken = level["app_token"];
  setting->privilege = StringToNumber<int32_t>(level["privilege"]);
  
  setting->pingInterval = StringToNumber<int32_t>(level["ping_interval"]);
  setting->tProtocal = StringToNumber<int32_t>(level["transport_protacal"]);
  setting->mProtocal = StringToNumber<int32_t>(level["method_protacal"]);
  
  if (setting->type == ST_Server)
  {
    setting->listenIp = level["listen_ip"];
    setting->listenPort = StringToNumber<int32_t>(level["listen_port"]);
  }
  else
  {
    setting->listenPort = 0;
  }

  if (RegisterDepencyApi(setting, level) != 0)
  {
    LOG(ERROR) << "parse depency api error: " << serviceName;
    return -1;
  }

  if (RegisterProvideApi(setting, level) != 0)
  {
    LOG(ERROR) << "parse provide api error: " << serviceName;
    return -1;
  }

  m_id2settings[setting->appId] = setting;
  return 0;
}

int ServiceSettingManager::RegisterDepencyApi(std::shared_ptr<ServiceSetting>& setting, INI::Level& level)
{
  for (size_t i = 0; i < 1000; ++i)
  {
    int r = RegisterDepencyApi(setting, level, i);
    if (r > 0)  
      break;
    else if (r < 0) 
      return -1;
  }
  return 0;
}

int ServiceSettingManager::RegisterProvideApi(std::shared_ptr<ServiceSetting>& setting, INI::Level& level)
{
  if (setting->type != ST_Server)
    return 0;

  for (size_t i = 0; i < 1000; ++i)
  {
    int r = RegisterProvideApi(setting, level, i);
    if (r > 0) 
      break;
    else if (r < 0) 
      return -1;
  }
  return 0;
}

int ServiceSettingManager::RegisterProvideApi(std::shared_ptr<ServiceSetting>& setting, INI::Level& level, int i)
{
  char buffer[64] = { 0 };
  std::string val;
  sprintf_s(buffer, 64, "provide_api%d", i);
  if (!level.getstring(buffer, val))
    return 1;
  ApiProvideSetting api(val);
  if (!api.isValid)
    return -1;
  setting->provide_apis.push_back(api);
  m_api2settings[api.apiName] = setting;
  return 0;
}

int ServiceSettingManager::RegisterDepencyApi(std::shared_ptr<ServiceSetting>& setting, INI::Level& level, int i)
{
  char buffer[64] = { 0 };
  std::string val;
  sprintf_s(buffer, 64, "depency_api%d", i);
  if (!level.getstring(buffer, val))
    return 1;
  ApiDepencySetting api(val);
  if (!api.isValid)
    return -1;
  setting->depency_apis.push_back(api);
  return 0;
}

std::shared_ptr<ServiceSetting>& ServiceSettingManager::GetServiceSettings(int32_t appId)
{
  static std::shared_ptr<ServiceSetting> null;
  auto it = m_id2settings.find(appId);
  if (it == m_id2settings.end())
    return null;

  return it->second;
}

std::shared_ptr<ServiceSetting>& ServiceSettingManager::GetServiceSettings(const std::string& methodName)
{
  static std::shared_ptr<ServiceSetting> null;
  auto it = m_api2settings.find(methodName);
  if (it == m_api2settings.end())
    return null;

  return it->second;
}

int ServiceSettingManager::UpdateAllRelation()
{
  for (auto it : m_id2settings)
  {
    if (UpdateRelation(it.second) != 0)
      return -1;
  }

  return 0;
}

int ServiceSettingManager::UpdateRelation(std::shared_ptr<ServiceSetting>& setting)
{
  for (auto it : setting->depency_apis)
  {
    if (UpdateRelationWithDepencyApi(setting, it) != 0)
      return -1;
  }

  return 0;
}

int ServiceSettingManager::UpdateRelationWithDepencyApi(std::shared_ptr<ServiceSetting>& setting, ApiDepencySetting& api)
{
  std::shared_ptr<ServiceSetting>& depency = GetServiceSettings(api.apiName);
  if (!depency)
  {
    LOG(ERROR) << "can't find depency api: " << api.apiName;
    return -1;
  }
  m_depencys[setting->appId].insert(depency->appId);
  m_provides[depency->appId].insert(setting->appId);
  return 0;
}

const std::set<int32_t>& ServiceSettingManager::GetDepencyIds(int32_t appId)
{
  static std::set<int32_t> null;
  auto it = m_depencys.find(appId);
  if (it == m_depencys.end())
    return null;

  return it->second;
}

const std::set<int32_t>& ServiceSettingManager::GetProvideIds(int32_t appId)
{
  static std::set<int32_t> null;
  auto it = m_provides.find(appId);
  if (it == m_provides.end())
    return null;

  return it->second;
}
