#ifndef TEST_API_H
#define TEST_API_H

#include "../property.h"
#include "../yyrpc.h"
#include <vector>
#include <string>

struct Request : public TRpcModelBase <Request>
{
  double memberDouble;
  std::string memberString;
  std::vector<int> memberVector;
  Request(double _memberDouble, std::string _memberString, std::vector<int> _memberVector): memberDouble(_memberDouble), memberString(_memberString), memberVector(_memberVector) {}
  Request() {}
};

YYRPC_MODEL_REGISTER(Request, memberDouble, memberString, memberVector)

bool TestMethod(const std::vector<int>& v, const std::string& s, double f, const Request& req);

#endif  //! #ifndef TEST_API_H
