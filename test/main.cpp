#include <iostream>
#include <string>
#include "testapi.h"
#include "test.h"

template <typename T> T xidentity(T);

template<class T>
std::vector<T> convert(int argument)
{
}

template<class T>
struct aaaa
{
  int begin() { return 0; }
};

using namespace std;

int main(int argc, char *argv[])
{
  //foo();

  std::stringstream s;
  std::string g;
  std::vector<int> v;
  Request r;
  Request a[6];

  serialization(s, a);
  serialization(s, r);
  serialization(s, v);
  serialization(s, g);
 /* serialization(s, 1);
  serialization(s, g);*/
  return 0;
}