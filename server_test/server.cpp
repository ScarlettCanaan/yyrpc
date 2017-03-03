#include <iostream>
#include <string>
#include "test_api.h"
#include "server.h"
#include "yyrpc.h"

int server_callee()
{
  SimpleAPi::Hello.bind([]()
  {
    std::cout << "Hello World!" << std::endl;
  });

  SimpleAPi::Sum.bind<any_thread_t>([](int a, int b) -> int
  {
    //std::cout << "sum:" << a + b << std::endl;
    return a + b;
  });

  TestAPi::TestMethod.bind([](
    const std::vector<int>& v,
    const std::string& s,
    double f,
    const Request& r) -> Response
  {
    std::cout << f << std::endl;
    Response rsp;
    rsp.memberMap[1] = "youjing";
    return rsp;
  });

  return 0;
}