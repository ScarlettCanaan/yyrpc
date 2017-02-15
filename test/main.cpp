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
  std::string g { "object" };
  std::vector<int> v { 1, 2, 3, 4, 5 };
  int intarray[] = { 6, 7, 8, 9, 10 };
  Request r ( 0.1f, "Adapter", { 1, 2, 3 } );

  serialization(s, intarray); //array type
  serialization(s, v);        //container type
  serialization(s, g);        //string type
  serialization(s, r);        //user define

  std::size_t offset = 0;
  std::cout << "Array type Offset: " << offset << std::endl;
  for (int i = 0; i < 5; ++i)
  {
    auto oh = msgpack::unpack(s.str().data(), s.str().size(), offset);
    auto obj = oh.get();
    
    std::cout << obj << " ";
    assert(obj.as<int>()== intarray[i]);
  }
  std::cout << std::endl << "Container type Offset: " << offset << std::endl;
  {
    auto oh = msgpack::unpack(s.str().data(), s.str().size(), offset);
    auto obj = oh.get();
    
    std::cout << obj << std::endl;
    assert(obj.as<decltype(v)>() == v);
  }
  std::cout << "string type Offset: " << offset << std::endl;
  {
    auto oh = msgpack::unpack(s.str().data(), s.str().size(), offset);
    auto obj = oh.get();
    
    std::cout << obj << std::endl;
    assert(obj.as<decltype(g)>() == g);
  }
  std::cout << "Request type Offset: " << offset << std::endl;
  for (int i = 0; i < 3; ++i)
  {
    auto oh = msgpack::unpack(s.str().data(), s.str().size(), offset);
    auto obj = oh.get();
    
    std::cout << obj << std::endl;
    //assert(obj.as<decltype(v)>() == v);
    //static_assert(obj.as<decltype(v)>() == v, "Deserialize mismatch.");
  }
  std::cout << "Success!" << std::endl;
  return 0;
}
