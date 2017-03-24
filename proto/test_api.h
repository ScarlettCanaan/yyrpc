#ifndef TEST_API_H
#define TEST_API_H

#include "orpc.h"
#include <vector>
#include <string>

enum Sex
{
  Male,
  Female,
};

struct Person
{
  int32_t person_id;  //support base type
  int8_t age;
  Sex sex;  //support enum
  std::string name; //support std::string
  ORPC_PROPERTY(person_id, age, sex, name);
};

struct Family
{
  int32_t family_id;
  std::string family_name;
  std::string address;
  std::vector<Person> members; //support std container
  ORPC_PROPERTY(family_id, family_name, address, members);
};

namespace FamilyApi
{
  ORPC_METHOD(Hello, void(void)); //NoReturnNoParam
  ORPC_METHOD(SetFamilyName, void(int32_t family_id, const std::string& family_name)); //NoReturnWithParam
  ORPC_METHOD(GetFamilyCount, int32_t(void)); //NoParamWithReturn
  ORPC_METHOD(GetFamilyName, std::string(int32_t family_id));  //SimpleReturnOneParam
  ORPC_METHOD(AddFamilyBatch, int32_t(const std::vector<Family>& familys)); //StlContainerReturnOrParam
  ORPC_METHOD(AddFamily, bool(const Family& family)); //UserDefineTypeReturnOrParam
  ORPC_METHOD(GetFamily, Family(int32_t family_id)); //UserDefineTypeReturnOrParam
  ORPC_METHOD(GetAllFamily, std::vector<Family>(void)); //StlContainerReturnOrParam
}

namespace FamilyEventApi
{
  ORPC_EVENT(Haha, void()); //NoParam
  ORPC_EVENT(FamilyNameChanged, void(int32_t family_id, const std::string& family_name)); //SimpleParam
  ORPC_EVENT(FamilyAdded, void(const Family& new_family)); //UserDefineTypeParam
}

#endif  //! #ifndef TEST_API_H