#include "uv.h"
#include "yyrpc.h"
#include "endpoint/endpoint_manager.h"
#include "ini/ini.hpp"
#include "listenpoint/listenpoint_manager.h"

bool InitClient(INI::Level& level)
{
  bool is_name_center = false;
  level.getbool("is_name_center", is_name_center);
  if (is_name_center)
    return true;

  std::string name_center_ip; 
  if (!level.getstring("name_center_ip", name_center_ip))
    return false;

  int32_t name_center_port;
  if (!level.getinteger("name_center_port", name_center_port))
    return false;

  int32_t app_id = 0;
  if (!level.getinteger("app_id", app_id))
    return false;

  std::string app_token;
  if (!level.getstring("app_token", app_token))
    return false;

  return true;
}

bool InitServer(INI::Level& level)
{
  int32_t listen_port = 0;
  level.getinteger("listen_port", listen_port);
  if (listen_port == 0)
    return true;

  int32_t protacal = 0;
  if (!level.getinteger("protacal", protacal))
    return false;

  std::shared_ptr<ListenPoint> p = ListenPointManager::GetInstance().Create("0.0.0.0", listen_port, (TransportProtocol)protacal);
  return p.get() != 0;
}

bool InitRpc(const char* cfgFile, OnRpcInited callback)
{
  INI::Parser parse(cfgFile);
  INI::Level::section_map_t& sections = parse.top().sections;
  INI::Level& level = sections["settings"];

  if (!InitClient(level))
    return false;

  return InitServer(level);
}

const std::shared_ptr<EndPoint>& QueryEndPoint(const std::string& api)
{
  return EndPointManager::GetInstance().QueryEndPoint(api);
}

const std::shared_ptr<EndPoint>& CreateEndPoint(const std::string& ip, int32_t port, TransportProtocol protocal, EndPointFlag flag)
{
  return EndPointManager::GetInstance().CreateEndPoint(ip, port, protocal, flag);
}
