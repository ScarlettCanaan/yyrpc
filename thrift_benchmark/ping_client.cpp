#include "ping.h"  // As an example

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace Test;

void GetTimeMillSecond(int64_t* curTime)
{
  if (!curTime)
    return;

  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> time_point;
  time_point = std::chrono::high_resolution_clock().now();
  int64_t t = time_point.time_since_epoch().count();
  t = t / 1000000;
  *curTime = t;
}

void CallTest(int iterCount)
{
  boost::shared_ptr<TSocket> socket(new TSocket("172.31.0.14", 9090));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  SomethingClient client(protocol);
  transport->open();

  for (int i = 0; i < iterCount; ++i)
  {
    int32_t p = client.ping();
    if (p != 0)
      std::cout << "p != 0, p: " << p << std::endl;
  }
  transport->close();
}

static size_t parse_int() {
  const char* arg = optarg;
  char* end;
  errno = 0;
  int64_t value = strtol(arg, &end, 10);
  if (errno != 0 || *end != '\0' || value <= 0) {
    fprintf(stderr, "requires a positive integer\n");
    exit(EXIT_FAILURE);
  }
  if (SIZE_MAX < INT64_MAX && value >(int64_t)SIZE_MAX) {
    fprintf(stderr, "argument is out of bounds\n");
    exit(EXIT_FAILURE);
  }
  return (size_t)value;
}

int main(int argc, char **argv) {
  int totalWork = 1000 * 1000;
  int workerNum = 1;

  int opt;
  while ((opt = getopt(argc, argv, "w:")) != -1) {
    switch (opt) {
    case 'w':
      workerNum = parse_int();
      break;
    case 't':
      totalWork = parse_int();
      break;
    default: /* ? */
      fprintf(stderr, "invalid option -- '%c'\n", optopt);
      return EXIT_FAILURE;
    }
  }

  if (optind < argc) {
    fprintf(stderr, "not an option -- \"%s\"\n", argv[optind]);
    return EXIT_FAILURE;
  }

  int64_t curTime = 0;
  GetTimeMillSecond(&curTime);
  std::cout << "begin connect time: " << curTime << std::endl;

  GetTimeMillSecond(&curTime);
  int64_t beginTime = curTime;

  std::cout << "begin bench time: " << curTime << std::endl;

  std::list<std::shared_ptr<std::thread>> threadList;
  for (int i = 0; i < workerNum; ++i)
    threadList.push_back(std::make_shared<std::thread>(std::bind(CallTest, totalWork / workerNum)));

  GetTimeMillSecond(&curTime);
  std::cout << "begin join time: " << curTime << std::endl;

  for (auto it : threadList)
    it->join();

  GetTimeMillSecond(&curTime);
  std::cout << "end join time: " << curTime << std::endl;
  std::cout << "tps." << totalWork * 1000.f / (curTime - beginTime) << std::endl;

  return 0;
}
