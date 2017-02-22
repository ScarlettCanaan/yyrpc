#include <iostream>
#include <string>
#include "test_api.h"
#include "client.h"
#include "yyrpc.h"

int client_caller()
{
  SimpleAPi::Hello();

  auto endpoint = CreateEndPoint("172.31.0.82", 1234, TP_TCP);
  SimpleAPi::Sum(endpoint, 1, 2)->on_result([](int sum)
  {
    std::cout << "sum:" << sum << std::endl;
  });

  Request r;
  r.memberDouble = 0.01f;
  r.memberString = "Adapter";
  std::vector<int> v;
  auto response = TestAPi::TestMethod(v, "", 1.0f, r);

  Response resp = response->wait();

  return 0;
}