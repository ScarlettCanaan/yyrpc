#include "uv.h"
#include "yyrpc.h"
#include "endpoint/endpoint_manager.h"
#include "ini/ini.hpp"
#include "listenpoint/listenpoint_manager.h"
#include "listenpoint/listenpoint.h"
#include "worker/server_worker_pool.h"
#include "worker/client_worker_pool.h"
#include "st.h"

static bool yyrpc_inited = false;

#define CHECK_STRING(section, key) \
std::string key;\
if (!section.getstring(#key, key))\
  return false;\

#define CHECK_INT(section, key) \
int32_t key = 0;\
if (!section.getinteger(#key, key))\
  return false;\

static bool InitAsClient(INI::Level& level)
{
  bool use_name_center = false;
  level.getbool("use_name_center", use_name_center);
  if (!use_name_center)
    return true;

  CHECK_STRING(level, name_center_ip);
  CHECK_INT(level, name_center_port);
  CHECK_INT(level, app_id);
  CHECK_STRING(level, app_token);

  EndPointWrapper wrapper = EndPointManager::GetInstance().CreateEndPoint(name_center_ip, name_center_port, TP_TCP, MP_HTTP);
  if (!wrapper)
    return false;

  return true;
}

static bool InitAsServer(INI::Level& level)
{
  int32_t listen_port = 4358;
  level.getinteger("listen_port", listen_port);

  int32_t protacal = 0;
  level.getinteger("protacal", protacal);

  int32_t method_protacal = 0;
  level.getinteger("method_protacal", method_protacal);

  ListenPointWrapper listenWrapper = ListenPointManager::GetInstance().CreateListenPoint("0.0.0.0", listen_port, (TransportProtocol)protacal, (MethodProtocol)method_protacal);
  if (!listenWrapper)
    return false;

  int try_count = 0;
  while (listenWrapper.endpoint->GetStatus() == LPS_INIT && try_count < 3)
  {
    ++try_count;
    uv_sleep(100);
  }

  if (listenWrapper.endpoint->GetStatus() != LPS_LISTEN_SUCC)
    return false;

  bool use_name_center = false;
  level.getbool("use_name_center", use_name_center);
  if (!use_name_center)
    return true;

  CHECK_STRING(level, name_center_ip);
  CHECK_INT(level, name_center_port);
  CHECK_INT(level, app_id);
  CHECK_STRING(level, app_token);

  EndPointWrapper connWrapper = EndPointManager::GetInstance().CreateEndPoint(name_center_ip, name_center_port, TP_TCP, MP_HTTP);
  if (!connWrapper)
    return false;

  return true;
}

static bool InitAsNameServer(INI::Level& level)
{
  int32_t listen_port = 4359;
  level.getinteger("listen_port", listen_port);

  int32_t protacal = 0;
  level.getinteger("protacal", protacal);

  ListenPointWrapper listenWrapper = ListenPointManager::GetInstance().CreateListenPoint("0.0.0.0", listen_port, (TransportProtocol)protacal, MP_HTTP);
  if (!listenWrapper)
    return false;

  int try_count = 0;
  while (listenWrapper.endpoint->GetStatus() == LPS_INIT && try_count < 3)
  {
    ++try_count;
    uv_sleep(100);
  }

  return listenWrapper.endpoint->GetStatus() == LPS_LISTEN_SUCC;
}

static bool g_use_fiber = false;

static bool NOTHROW InitFiberPool(INI::Level& level)
{
  if (st_init() != 0)
  {
    LOG(ERROR) << "st_init failed.";
    return false;
  }

  int32_t max_callee_fiber_num = 0;
  level.getinteger("max_callee_fiber_num", max_callee_fiber_num);
  if (max_callee_fiber_num < 1000)
    max_callee_fiber_num = 1000;

  CallerManager::GetInstance().Init(g_use_fiber, max_callee_fiber_num);
  CalleeManager::GetInstance().Init(g_use_fiber, max_callee_fiber_num);
  return true;
}

static bool InitThreadPool(INI::Level& level)
{
  int32_t client_worker_num = 1;
  level.getinteger("client_worker_num", client_worker_num);
  if (client_worker_num < 1)
    client_worker_num = 1;
  ClientWorkerPool::GetInstance().Init(client_worker_num);

  int32_t server_worker_num = 1;
  level.getinteger("server_worker_num", server_worker_num);
  if (server_worker_num < 1)
    server_worker_num = 1;
  ServerWorkerPool::GetInstance().Init(server_worker_num);

  CallerManager::GetInstance().Init(false, 0);
  CalleeManager::GetInstance().Init(false, 0);
  return true;
}

bool InitRpc(const char* cfgFile)
{
  INI::Parser parse(cfgFile);
  INI::Level::section_map_t& sections = parse.top().sections;
  INI::Level& level = sections["settings"];

  //init run mode
  std::string run_mode = "client";
  level.getstring("run_mode", run_mode);

  if (run_mode == "client")
  {
    if (!InitAsClient(level))
    {
      LOG(ERROR) << "InitAsClient failed.";
      return false;
    }
  }
  else if (run_mode == "server")
  {
    if (!InitAsServer(level))
    {
      LOG(ERROR) << "InitAsServer failed.";
      return false;
    }
  }
  else if (run_mode == "nameserver")
  {
    if (!InitAsNameServer(level))
    {
      LOG(ERROR) << "InitAsNameServer failed.";
      return false;
    }
  }
  else
  {
    LOG(ERROR) << "InitRpc unknown run_mode." << run_mode;
    return false;
  }
  
  //init work pool
  level.getbool("use_fiber", g_use_fiber);
  if (g_use_fiber)
  {
    if (!InitFiberPool(level))
      return false;
  }
  else if (!InitThreadPool(level))
  {
    return false;
  }

  yyrpc_inited = true;
  return true;
}

bool RunRpc()
{
  if (!yyrpc_inited)
    return false;

  CallerManager::GetInstance().PumpMessage();
  CalleeManager::GetInstance().PumpMessage();

  if (g_use_fiber)
    st_usleep(1);

  return true;
}

bool UnInitRpc()
{
  if (!yyrpc_inited)
    return false;

  CalleeManager::GetInstance().UnInit();
  CallerManager::GetInstance().UnInit();
  ServerWorkerPool::GetInstance().UnInit();
  ClientWorkerPool::GetInstance().UnInit();
  yyrpc_inited = false;
  return true;
}

EndPointWrapper QueryEndPoint(const std::string& api)
{
  return EndPointManager::GetInstance().QueryEndPoint(api);
}

EndPointWrapper CreateEndPoint(const std::string& ip, int32_t port, TransportProtocol tProtocal, MethodProtocol mProtocal)
{
  return EndPointManager::GetInstance().CreateEndPoint(ip, port, tProtocal, mProtocal);
}

int CreateClientFiber(const std::function<int(void)>& f)
{
  return CallerManager::GetInstance().CreateFiber(f);
}
