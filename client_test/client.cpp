#include <iostream>
#include <string>
#include "test_api.h"
#include "client.h"
#include "yyrpc.h"

int client_caller()
{
  SimpleAPi::Hello();

  auto endpoint = CreateEndPoint("127.0.0.1", 1234, TP_TCP);
  SimpleAPi::Sum(endpoint, 1, 2).then<any_thread_t>(
  [](int sum)
  {
    std::cout << "sum:" << sum << std::endl;
  },
  [](int error)
  {
    std::cout << "error:" << error << std::endl;
  }
  );

  Request r;
  r.memberDouble = 0.01f;
  r.memberString = "Adapter";
  std::vector<int> v;
  auto response = TestAPi::TestMethod(endpoint, v, "", 1.0f, r);

  try
  {
    Response resp = response.wait();
    std::cout << resp.memberMap[1] << std::endl;
  }
  catch (std::runtime_error& e)
  {
    std::cout << "runtime_error:" << e.what() << std::endl;
  }
  
  return 0;
}

int do_call(EndPointWrapper& wrpper)
{
  try
  {
    int resp = SimpleAPi::Sum(wrpper, 1, 2);
    if (resp != 3)
      std::cout << "resp != 3" << std::endl;
  }
  catch (std::runtime_error& e)
  {
    std::cout << "runtime_error:" << e.what() << std::endl;
  }
  return 0;
}

int bench_single_thread_call()
{
  auto endpoint = CreateEndPoint("127.0.0.1", 4358, TP_TCP);
  int64_t t1;
  GetTimeMillSecond(&t1);
  for (int i = 0; i < 100000; ++i)
  {
    do_call(endpoint);
    if (i % 10000 != 0)
      continue;
    int64_t t3;
    GetTimeSecond(&t3);
    std::cout << i << " time:" << t3 << std::endl;
  }
  int64_t t2;
  GetTimeMillSecond(&t2);

  std::cout << "time:" << t2 - t1 << std::endl;
  return 0;
}

int count = 0;

#ifdef WIN32
int NOTHROW do_tiny_call(EndPointWrapper& wrpper)
{
  int resp = SimpleAPi::Sum(wrpper, 1, 2);
  ++count;
  if (resp != 3)
    std::cout << "resp != 3" << std::endl;

  if (count % 10000 == 0)
  {
    int64_t t2;
    GetTimeMillSecond(&t2);
    std::cout << "count:" << count << " time:" << t2 << std::endl;
  }
    
  return 0;
}

#else
int NOTHROW do_tiny_call(EndPointWrapper& wrpper)
{
  try
  {
    int resp = SimpleAPi::Sum(wrpper, 1, 2);
    ++count;
    if (resp != 3)
      std::cout << "resp != 3" << std::endl;
  }
  catch (std::runtime_error& e)
  {
    std::cout << "runtime_error:" << e.what() << std::endl;
  }

  if (count % 10000 == 0)
  {
    int64_t t2;
    GetTimeMillSecond(&t2);
    std::cout << "count:" << count << " time:" << t2 << std::endl;
  }

  return 0;
}
#endif

int fiber_batch_tiny_call(EndPointWrapper& wrpper)
{
  for (int i = 0; i < 100; ++i)
    do_tiny_call(wrpper);
  return 0;
}

int bench_tiny_call()
{
  auto endpoint = CreateEndPoint("127.0.0.1", 4358, TP_TCP);
  int64_t t1;
  GetTimeMillSecond(&t1);

  for (int i = 0; i < 1000; ++i)
    CreateClientFiber(std::bind(fiber_batch_tiny_call, endpoint));

  int64_t t2;
  GetTimeMillSecond(&t2);

  std::cout << "time:" << t2 - t1 << std::endl;
  return 0;
}
