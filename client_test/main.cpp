#include <iostream>
#include <string>
#include "client.h"
#include "yyrpc.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (!InitRpc("./config.cfg"))
    return -1;
  
  bench_call();
  //client_caller();

  while (true)
  {
    RunRpc();
    uv_sleep(1);
  }

  UnInitRpc();

  return 0;
}