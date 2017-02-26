#include "uv.h"
#include "yyrpc.h"
#include "endpoint/endpoint_manager.h"
#include "ini/ini.hpp"
#include "listenpoint/listenpoint_manager.h"
#include "listenpoint/listenpoint.h"
#include "worker/server_worker_pool.h"
#include "worker/client_worker_pool.h"

static bool yyrpc_inited = false;

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

  EndPointWrapper wrapper = EndPointManager::GetInstance().CreateEndPoint(name_center_ip, name_center_port, TP_TCP);
  if (!wrapper)
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

  ListenPointWrapper wrapper = ListenPointManager::GetInstance().CreateListenPoint("0.0.0.0", listen_port, (TransportProtocol)protacal);
  if (!wrapper)
    return false;

  int try_count = 0;
  while (wrapper.endpoint->GetStatus() == LPS_INIT && try_count < 3)
  {
    ++try_count;
    uv_sleep(100);
  }

  return wrapper.endpoint->GetStatus() == LPS_LISTEN_SUCC;
}

bool InitRpc(const char* cfgFile)
{
  INI::Parser parse(cfgFile);
  INI::Level::section_map_t& sections = parse.top().sections;
  INI::Level& level = sections["settings"];

  if (!InitClient(level))
    return false;

  if (!InitServer(level))
    return false;
  
  int32_t client_worker_num = 0;
  level.getinteger("client_worker_num", client_worker_num);
  if (client_worker_num < 1)
    client_worker_num = 1;
  ClientWorkerPool::GetInstance().Init(client_worker_num);

  int32_t server_worker_num = 0;
  level.getinteger("server_worker_num", server_worker_num);
  if (server_worker_num < 1)
    server_worker_num = 1;
  ServerWorkerPool::GetInstance().Init(server_worker_num);

  yyrpc_inited = true;
  return true;
}

bool RunRpc()
{
  if (!yyrpc_inited)
    return false;

  CallerManager::GetInstance().PumpMessage();
  CalleeManager::GetInstance().PumpMessage();

  return true;
}

bool UnInitRpc()
{
  if (!yyrpc_inited)
    return false;

  ClientWorkerPool::GetInstance().UnInit();
  ServerWorkerPool::GetInstance().UnInit();
  yyrpc_inited = false;
  return true;
}

EndPointWrapper QueryEndPoint(const std::string& api)
{
  return EndPointManager::GetInstance().QueryEndPoint(api);
}

EndPointWrapper CreateEndPoint(const std::string& ip, int32_t port, TransportProtocol protocal)
{
  return EndPointManager::GetInstance().CreateEndPoint(ip, port, protocal);
}
