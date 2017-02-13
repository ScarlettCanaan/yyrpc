#include <iostream>
#include <string>
#include "testapi.h"
#include "test.h"
#include "../yyrpc.h"

int foo()
{
  Request r;
  r.memberDouble = 0.01f;
  r.memberString = "Adapter";

  std::vector<int> v;

  //SYNC_CALL(TestMethod, v, "", 1.0f, r);

  return 0;
}