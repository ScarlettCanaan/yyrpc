#include <iostream>
#include <string>
#include "orpc.h"
#include "server.h"
#include "rpc_application.h"

using namespace std;

int main(int argc, char *argv[])
{
  NameServerApp app("./config.cfg");
  app.Run(ART_DEFAULT);

  return 0;
}