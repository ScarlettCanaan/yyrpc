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

#ifndef ORPC_APPLICATION_H_
#define ORPC_APPLICATION_H_

#include "build_config.h"
#include "util/ini.hpp"
#include "property.h"
#include "event.h"
#include "method.h"
#include <memory>
#include "stub/connection_monitor.h"

_START_ORPC_NAMESPACE_

class TcpPeer;

enum AppRunType
{
  ART_DEFAULT,
  ART_ONCE,
};

class RpcApplication
{
public:
  RpcApplication();
  virtual ~RpcApplication();
public:
  bool NOTHROW Init(const char* cfgFile);
  bool Run(AppRunType type = ART_DEFAULT);
  bool UnInit();
private:
#ifdef ORPC_USE_FIBER 
  bool NOTHROW InitFiberPool(INI::Level& level);
#else
  bool InitThreadPool(INI::Level& level);
#endif  //! #ifdef ORPC_USE_FIBER

  bool RunOnce();
  void RunDefaultLoop();
public:
  virtual bool OnIdle() { return true; }
protected:
  virtual bool DoInit(INI::Level& level) = 0;
private:
  bool m_bInited;
};

class RpcClientApplication : public RpcApplication
{
protected:
  virtual bool DoInit(INI::Level& level) override;
public:
  static EndPointWrapper QueryEndPoint(const std::string& api);
  static EndPointWrapper CreateEndPoint(const std::string& ip, int32_t port, int32_t flags);
  static bool CheckEndpointAllReady(uint32_t timeoutMs = -1);
#ifdef ORPC_USE_FIBER 
  static int CreateClientFiber(const std::function<int(void)>& f);
#endif  //! #ifdef ORPC_USE_FIBER
};

class RpcServerApplication : public RpcClientApplication
{
protected:
  virtual bool DoInit(INI::Level& level) override;
public:
  static void SetConnectMonitor(ConnectionMonitor* monitor);
};

class RpcNameCenterApplication : public RpcServerApplication
{
protected:
  virtual bool DoInit(INI::Level& level) override;
public:
  static void SetEventDispatcher(EventDispatcher* disp);
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_APPLICATION_H_
