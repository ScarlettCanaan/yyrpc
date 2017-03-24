#include <iostream>
#include <string>
#include "server.h"
#include "rpc_application.h"

using namespace std;
using namespace orpc;

int main(int argc, char *argv[])
{
  ServerApp app("./config.cfg");
  app.Run(ART_DEFAULT);

  return 0;
}