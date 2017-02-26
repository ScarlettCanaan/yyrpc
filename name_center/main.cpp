#include <iostream>
#include <string>
#include "yyrpc.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (!InitRpc("./config.cfg"))
    return -1;

  getchar();
  return 0;
}