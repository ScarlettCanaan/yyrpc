#include <iostream>
#include <string>
#include "../proto/test_api.h"
#include "thread_client_bench.h"
#include "orpc.h"
#include "rpc_application.h"
#include "glog/logging.h"

using namespace orpc;

ThreadClientBenchApp::ThreadClientBenchApp(const char* cfgFile, int32_t workerNum, int32_t totalWork, bool isMultiConn)
{
  bool bInited = RpcClientApplication::Init(cfgFile);
  if (!bInited)
    throw std::runtime_error("failed to init rpc.");

  Init(cfgFile, workerNum, totalWork, isMultiConn);
}

int ThreadClientBenchApp::Init(const char* cfgFile, int32_t workerNum, int32_t totalWork, bool isMultiConn)
{
  int64_t curTime = 0;
  GetTimeMillSecond(&curTime);
  LOG(ERROR) << "begin connect time: " << curTime;

  std::vector<EndPointWrapper> m_endPoints;
  int32_t flags = TP_TCP << 8 | MP_BINARY;
  if (isMultiConn)
    flags |= MP_FORCE_CREATE;
  if (workerNum == 1)
    flags |= MP_SYNC_CLIENT;

  for (int i = 0; i < workerNum; ++i)
    m_endPoints.push_back(RpcClientApplication::CreateEndPoint("172.31.0.14", 4358, flags));

  CheckEndpointAllReady();

  GetTimeMillSecond(&curTime);
  int64_t beginTime = curTime;

  LOG(ERROR) << "begin bench time: " << curTime;

  std::list<std::shared_ptr<std::thread>> threadList;
  for (int i = 0; i < workerNum; ++i)
    threadList.push_back(std::make_shared<std::thread>(std::bind(&ThreadClientBenchApp::CallTest, this, m_endPoints[i], totalWork / workerNum)));

  GetTimeMillSecond(&curTime);
  LOG(ERROR) << "begin join time: " << curTime;

  for (auto it : threadList)
    it->join();

  GetTimeMillSecond(&curTime);
  LOG(ERROR) << "end join time: " << curTime;
  LOG(ERROR) << "tps." << totalWork * 1000.f / (curTime - beginTime) << std::endl;

  return 0;
}

int ThreadClientBenchApp::UnInit()
{
  return 0;
}

void ThreadClientBenchApp::CallTest(EndPointWrapper& wrapper, int iterCount)
{
  try
  {
    for (int i = 0; i < iterCount; ++i)
    {
      int32_t count = FamilyApi::GetFamilyCount(wrapper);
      assert(count == 0);
    }
  }
  catch (std::runtime_error& e)
  {
    std::cout << "FamilyApi::GetFamilyCount runtime_error:" << e.what() << std::endl;
  }
}
