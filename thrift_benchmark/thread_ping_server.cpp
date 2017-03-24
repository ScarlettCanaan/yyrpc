#include "ping.h"
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <server/TThreadPoolServer.h>
#include <transport/TServerSocket.h>
#include <transport/TTransportUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using boost::shared_ptr;

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>

#include <boost/make_shared.hpp>
#include <stdio.h>
#include <unistd.h>

using namespace Test;

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

class SomethingHandler : virtual public SomethingIf {
 public:
  SomethingHandler() {
   // Your initialization goes here
  }

  int32_t ping() {
    // Your implementation goes here
    return 0;
  }
};

class SomethingCloneFactory : virtual public SomethingIfFactory {
public:
  virtual ~SomethingCloneFactory() {}
  virtual SomethingIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo)
  {
    boost::shared_ptr<TSocket> sock = boost::dynamic_pointer_cast<TSocket>(connInfo.transport);
    return new SomethingHandler;
  }
  virtual void releaseHandler(SomethingIf* handler) {
    delete handler;
  }
};

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
  int workerCount = 4;
  int opt;
  while ((opt = getopt(argc, argv, "w:")) != -1) {
    switch (opt) {
    case 'w':
      workerCount = parse_int();
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

  boost::shared_ptr<ThreadManager> threadManager =
    ThreadManager::newSimpleThreadManager(workerCount);
  threadManager->threadFactory(
    boost::make_shared<PlatformThreadFactory>());
  threadManager->start();

  int port = 9090;
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TThreadPoolServer server(boost::make_shared<SomethingProcessorFactory>(boost::make_shared<SomethingCloneFactory>()), serverTransport, transportFactory, protocolFactory, threadManager);
  server.serve();
  return 0;
}
