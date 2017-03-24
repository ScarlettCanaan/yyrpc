#ifndef ORPC_CLIENT_H_
#define ORPC_CLIENT_H_

#include "build_config.h"
#include "rpc_application.h"
#include "../proto/test_api.h"

using namespace orpc;

class ClientApp : public RpcClientApplication
{
public:
  ClientApp(const char* cfgFile);
private:
  int Init(const char* cfgFile);
  int UnInit();
private:  //see test_api.h
  void CallTest();

  void SyncCallDemo();
  void AsyncCallDemo();
  void CallbackResultDemo();

private:
  void OnGetAllFamilysThisThread(const std::vector<Family>& fimilys);
  void OnGetAllFamilysAnyThread(const std::vector<Family>& fimilys);
  void OnGetAllFamilysError(int32_t error);

  void SubscribeEventNoParam();
  void SubscribeEventSimpleParam();
  void SubscribeEventUserDefineTypeParam();

  //void OnHaha(); //NoParam  ->>> lambda
  void OnFamilyNameChanged(int32_t family_id, const std::string& family_name); //SimpleParam
  void OnFamilyAdded(const Family& new_family); //UserDefineTypeParam
private:
  std::thread::id m_mainThreadId;
  std::vector<Family> m_familys;
};

#endif //! #ifndef ORPC_CLIENT_H_
