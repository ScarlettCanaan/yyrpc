#ifndef ORPC_THREAD_CLIENT_BENCH_H_
#define ORPC_THREAD_CLIENT_BENCH_H_

#include "build_config.h"
#include "rpc_application.h"
#include "../proto/test_api.h"

using namespace orpc;

class ThreadClientBenchApp : public RpcClientApplication
{
public:
  ThreadClientBenchApp(const char* cfgFile, int32_t workerNum, int32_t totalWork, bool isMultiConn);
private:
  int Init(const char* cfgFile, int32_t workerNum, int32_t totalWork, bool isMultiConn);
  int UnInit();
private:  //see test_api.h
  void CallTest(EndPointWrapper& wrapper, int iterCount);
};

#endif //! #ifndef ORPC_THREAD_CLIENT_BENCH_H_
