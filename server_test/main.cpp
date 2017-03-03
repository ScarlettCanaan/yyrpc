﻿#include <iostream>
#include <string>
#include "server.h"
#include "yyrpc.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (!InitRpc("./config.cfg"))
    return -1;

  server_callee();

  while (true)
  {
    RunRpc();
  }

  UnInitRpc();

  return 0;
}