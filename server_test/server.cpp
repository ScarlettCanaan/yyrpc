#include <iostream>
#include <string>
#include "test_api.h"
#include "server.h"
#include "yyrpc.h"

int server_callee()
{
  SimpleAPi::Hello.bind([]() -> bool
  {
    std::cout << "Hello World!" << std::endl;
    return true;
  });

  SimpleAPi::Sum.bind([](int a, int b) -> bool
  {
    std::cout << "sum:" << a + b << std::endl;
    return true;
  });

  TestAPi::TestMethod.bind([](
    const std::vector<int>& v,
    const std::string& s,
    double f,
    const Request& r) -> bool
  {
    std::cout << f << std::endl;
    return true;
  });

  return 0;
}