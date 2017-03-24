#ifndef ORPC_SERVER_H_
#define ORPC_SERVER_H_

#include "rpc_application.h"
#include "../proto/test_api.h"

using namespace orpc;

class ServerApp : public RpcServerApplication
{
public:
  ServerApp(const char* cfgFile);
private:
  int Init(const char* cfgFile);
  int UnInit();
private:  //see test_api.h
  void BindNoReturnNoParam();

  void BindNoReturnWithParam();
  void BindNoParamWithReturn();
  void BindSimpleReturnOneParam();
  void BindStlContainerReturnOrParam();
  void BindUserDefineTypeReturnOrParam();

  void FireEventNoParam();
  void FireEventSimpleParam(int32_t family_id, const std::string& family_name);
  void FireEventUserDefineTypeParam(const Family& new_family);
private:
  void OnSetFamilyName(int32_t family_id, const std::string& new_address); //NoReturnWithParam
  int32_t OnGetFamilyCount(void); //NoParamWithReturn
  std::string OnGetFamilyName(int32_t family_id);  //SimpleReturnOneParam
  int32_t OnAddFamilyBatch(const std::vector<Family>& familys); //StlContainerReturnOrParam
  bool OnAddFamily(const Family& family); //UserDefineTypeReturnOrParam
  Family OnGetFamily(int32_t family_id); //UserDefineTypeReturnOrParam
  std::vector<Family> OnGetAllFamily(void); //StlContainerReturnOrParam
private:
  std::thread::id m_mainThreadId;
  std::unordered_map<int32_t, Family> m_familys;
};

#endif //! #ifndef ORPC_SERVER_H_
