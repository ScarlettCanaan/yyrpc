#ifndef ORPC_FIBER_CLIENT_BENCH_H_
#define ORPC_FIBER_CLIENT_BENCH_H_

#include "build_config.h"
#include "rpc_application.h"
#include "../proto/test_api.h"

using namespace orpc;

class FiberClientBenchApp : public RpcClientApplication
{
public:
  FiberClientBenchApp(const char* cfgFile);
private:
  int Init(const char* cfgFile);
  int UnInit();
private:  //see test_api.h
  void CallTest();
};

#endif //! #ifndef ORPC_FIBER_CLIENT_BENCH_H_
