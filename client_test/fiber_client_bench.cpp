#include <iostream>
#include <string>
#include "../proto/test_api.h"
#include "fiber_client_bench.h"
#include "orpc.h"
#include "rpc_application.h"

using namespace orpc;

FiberClientBenchApp::FiberClientBenchApp(const char* cfgFile)
{
  bool bInited = RpcClientApplication::Init(cfgFile);
  if (!bInited)
    throw std::runtime_error("failed to init rpc.");
}

int FiberClientBenchApp::Init(const char* cfgFile)
{
  CallTest();
  return 0;
}

int FiberClientBenchApp::UnInit()
{
  return 0;
}

void FiberClientBenchApp::CallTest()
{

}
