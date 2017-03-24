/*
* The MIT License (MIT)
*
* Copyright (c) 2017-2018 youjing@yy.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "uv.h"
#include "rpc_application.h"
#include "endpoint/endpoint_manager.h"
#include "listenpoint/listenpoint_manager.h"
#include "listenpoint/listenpoint.h"
#include "worker/server_worker_pool.h"
#include "worker/client_worker_pool.h"
#include "../proto/common_api.h"

#define CHECK_STRING(section, key) \
std::string key;\
if (!section.getstring(#key, key))\
  return false;\

#define CHECK_INT(section, key) \
int32_t key = 0;\
if (!section.getinteger(#key, key))\
  return false;\

_START_ORPC_NAMESPACE_

RpcApplication::RpcApplication()
{
  m_bInited = false;
}

RpcApplication::~RpcApplication()
{
  UnInit();
}

#ifdef ORPC_USE_FIBER 
bool RpcApplication::InitFiberPool(INI::Level& level)
{
  if (st_init() != 0)
  {
    ORPC_LOG(ERROR) << "st_init failed.";
    return false;
  }

  int32_t max_callee_fiber_num = 0;
  level.getinteger("max_callee_fiber_num", max_callee_fiber_num);
  if (max_callee_fiber_num < 64)
    max_callee_fiber_num = 64;

  CallerManager::GetInstance().SetFiberInfo(true, max_callee_fiber_num);
  CalleeManager::GetInstance().SetFiberInfo(true, max_callee_fiber_num);
  return true;
}
#else
bool RpcApplication::InitThreadPool(INI::Level& level)
{
  int32_t client_worker_num = 1;
  level.getinteger("client_worker_num", client_worker_num);
  if (client_worker_num < 1)
    client_worker_num = 1;
  CallerManager::GetInstance().SetThreadInfo(client_worker_num);

  int32_t server_worker_num = 1;
  level.getinteger("server_worker_num", server_worker_num);
  if (server_worker_num < 1)
    server_worker_num = 1;
  CalleeManager::GetInstance().SetThreadInfo(server_worker_num);

  return true;
}
#endif  //! #ifdef ORPC_USE_FIBER

bool RpcApplication::Init(const char* cfgFile)
{
  if (m_bInited)
    return true;

  INI::Parser parse(cfgFile);
  INI::Level::section_map_t& sections = parse.top().sections;
  INI::Level& level = sections["settings"];

#ifndef ORPC_USE_FIBER 
  if (!InitThreadPool(level))
  {
    ORPC_LOG(ERROR) << "InitThreadPool failed.";
    return false;
  }
#endif //! #ifndef ORPC_USE_FIBER 

  CallerManager::GetInstance().Init();
  CalleeManager::GetInstance().Init();

  if (!DoInit(level))
  {
    ORPC_LOG(ERROR) << "DoInit failed.";
    return false;
  }

#ifdef ORPC_USE_FIBER
  if (!InitFiberPool(level))
  {
    ORPC_LOG(ERROR) << "InitFiberPool failed.";
    return false;
  }
#endif  //! #ifdef ORPC_USE_FIBER

  m_bInited = true;
  return true;
}

bool RpcApplication::RunOnce()
{
  CallerManager::GetInstance().PumpMessage();
  CalleeManager::GetInstance().PumpMessage();

  return OnIdle();
}

void RpcApplication::RunDefaultLoop()
{
#ifdef ORPC_USE_FIBER 
  while (RunOnce())
    st_usleep(1);
#else 
  while (RunOnce())
    std::this_thread::sleep_for(std::chrono::microseconds(1));
#endif  //! #ifdef ORPC_USE_FIBER 

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

bool RpcApplication::Run(AppRunType type)
{
  if (!m_bInited)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return false;
  }

  if (type == ART_DEFAULT)
    RunDefaultLoop();
  else if (type == ART_ONCE)
    RunOnce();

  return true;
}

bool RpcApplication::UnInit()
{
  CalleeManager::GetInstance().UnInit();
  CallerManager::GetInstance().UnInit();
  
  m_bInited = false;
  return true;
}

bool RpcClientApplication::DoInit(INI::Level& level)
{
  std::string run_mode = "client";
  level.getstring("run_mode", run_mode);
  if (run_mode != "client")
  {
    ORPC_LOG(ERROR) << "InitRpc client failed with run_mode." << run_mode;
    return false;
  }

  bool bUseNameCenter = false;
  level.getbool("use_name_center", bUseNameCenter);
  if (!bUseNameCenter)
    return true;

  CHECK_STRING(level, name_center_ip);
  CHECK_INT(level, name_center_port);
  CHECK_INT(level, app_id);
  CHECK_STRING(level, app_token);

  //name-center only support TP_TCP & MP_HTTP
  EndPointWrapper wrapper = CallerManager::GetInstance().CreateEndPoint(name_center_ip, name_center_port, TP_TCP << 8 | MP_HTTP);
  if (!wrapper)
    return false;

  auto r = NameCenterAPi::HelloClientService(wrapper, app_id, app_token);
  try
  {
    const HelloClientResponse& helloRsp = r.Wait();
    CallerManager::GetInstance().OnHelloResponse(app_id, helloRsp);
  }
  catch (std::runtime_error& e)
  {
    ORPC_LOG(ERROR) << "NameCenterAPi::HelloClientService error: " << e.what();
    return false;
  }

  CallerManager::GetInstance().SetNameCenter(wrapper);
  return true;
}

EndPointWrapper RpcClientApplication::QueryEndPoint(const std::string& api)
{
  return CallerManager::GetInstance().QueryEndPoint(api);
}

EndPointWrapper RpcClientApplication::CreateEndPoint(const std::string& ip, int32_t port, int32_t flags)
{
  return CallerManager::GetInstance().CreateEndPoint(ip, port, flags);
}

bool RpcClientApplication::CheckEndpointAllReady(uint32_t timeoutMs)
{
  return CallerManager::GetInstance().CheckEndpointAllReady(timeoutMs);
}

#ifdef ORPC_USE_FIBER
int RpcClientApplication::CreateClientFiber(const std::function<int(void)>& f)
{
  return CallerManager::GetInstance().CreateFiber(f);
}
#endif  //! #ifdef ORPC_USE_FIBER

bool RpcServerApplication::DoInit(INI::Level& level)
{
  std::string run_mode = "server";
  level.getstring("run_mode", run_mode);
  if (run_mode != "server")
  {
    ORPC_LOG(ERROR) << "InitRpc server failed with run_mode." << run_mode;
    return false;
  }

  int32_t listen_port = 4358;
  level.getinteger("listen_port", listen_port);

  int32_t protacal = 1;
  level.getinteger("transport_protacal", protacal);
  if (protacal != TP_TCP && protacal != TP_UDP)
  {
    ORPC_LOG(ERROR) << "transport_protacal only support TP_TCP(1) and TP_UDP(2), current: " << protacal;
    return false; 
  }

  int32_t method_protacal = 1;
  level.getinteger("method_protacal", method_protacal);
  if (protacal != MP_HTTP && protacal != MP_BINARY)
  {
    ORPC_LOG(ERROR) << "transport_protacal only support MP_HTTP(1) and MP_BINARY(2) current: " << method_protacal;
    return false;
  }

  ListenPointWrapper listenWrapper = CalleeManager::GetInstance().CreateListenPoint("0.0.0.0", listen_port, protacal << 8 | method_protacal);
  if (!listenWrapper)
    return false;

  int try_count = 0;
  while (listenWrapper.endpoint->GetStatus() == LPS_INIT && try_count < 3)
  {
    ++try_count;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (listenWrapper.endpoint->GetStatus() != LPS_LISTEN_SUCC)
    return false;

  bool bUseNameCenter = false;
  level.getbool("use_name_center", bUseNameCenter);
  if (!bUseNameCenter)
    return true;

  CHECK_STRING(level, name_center_ip);
  CHECK_INT(level, name_center_port);
  CHECK_INT(level, app_id);
  CHECK_STRING(level, app_token);

  //name-center only support TP_TCP & MP_HTTP
  EndPointWrapper connWrapper = CallerManager::GetInstance().CreateEndPoint(name_center_ip, name_center_port, TP_TCP << 8 | MP_HTTP);
  if (!connWrapper)
    return false;

  auto r = NameCenterAPi::HelloServerService(connWrapper, app_id, app_token);
  try
  {
    const HelloServerResponse& helloRsp = r.Wait();
    CalleeManager::GetInstance().OnHelloResponse(helloRsp.clientInfos, helloRsp.methods);
    CallerManager::GetInstance().OnHelloResponse(app_id, helloRsp.clientResponse);
  }
  catch (std::runtime_error& e)
  {
    ORPC_LOG(ERROR) << "NameCenterAPi::HelloServerService: " << e.what();
    return false;
  }

  CallerManager::GetInstance().SetNameCenter(connWrapper);
  return true;
}

void RpcServerApplication::SetConnectMonitor(ConnectionMonitor* monitor)
{
  CalleeManager::GetInstance().SetConnectMonitor(monitor);
}

bool RpcNameCenterApplication::DoInit(INI::Level& level)
{
  std::string run_mode = "name_center";
  level.getstring("run_mode", run_mode);
  if (run_mode != "name_center")
  {
    ORPC_LOG(ERROR) << "InitRpc name_center failed with run_mode." << run_mode;
    return false;
  }

  int32_t listen_port = 4359;
  level.getinteger("listen_port", listen_port);

  int32_t protacal = 0;
  level.getinteger("protacal", protacal);

  ListenPointWrapper listenWrapper = CalleeManager::GetInstance().CreateListenPoint("0.0.0.0", listen_port, protacal << 8 | MP_HTTP);
  if (!listenWrapper)
    return false;

  int try_count = 0;
  while (listenWrapper.endpoint->GetStatus() == LPS_INIT && try_count < 3)
  {
    ++try_count;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return listenWrapper.endpoint->GetStatus() == LPS_LISTEN_SUCC;
}

void RpcNameCenterApplication::SetEventDispatcher(EventDispatcher* disp)
{
  CalleeManager::GetInstance().SetEventDispatcher(disp);
}

_END_ORPC_NAMESPACE_
