#include <iostream>
#include <string>
#include "client.h"
#include "rpc_application.h"
#include "glog/logging.h"
#include "util/base_util.h"
#include "gflags/gflags.h"
#include "util/file_system.h"
#include "thread_client_bench.h"

using namespace std;
using namespace orpc;

DEFINE_int32(mode, 1, "spec mode for test");
DEFINE_int32(num, 100, "spec mode for test");
DEFINE_int32(total, 100 * 1000, "spec mode for test");
DEFINE_bool(multi_conn, true, "spec mode for test");

int main(int argc, char *argv[])
{
  if (!fs::PathExists("./log") && !fs::CreateAllDirs("./log"))
    return -1;

  google::InitGoogleLogging("");
  google::SetLogDestination(google::GLOG_INFO, "./log/kvclient_test_");

  google::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_mode == 0)
  {
    ClientApp app("./config.cfg");
    app.Run(ART_DEFAULT);
  }
  else if (FLAGS_mode == 1)
  {
    ThreadClientBenchApp app("./config.cfg", FLAGS_num, FLAGS_total, FLAGS_multi_conn);
    app.Run(ART_DEFAULT);
  }

  return 0;
}