#include <iostream>
#include <string>
#include "../proto/test_api.h"
#include "client.h"
#include "orpc.h"
#include "rpc_application.h"
#include <assert.h>
#include "glog/logging.h"

using namespace orpc;

ClientApp::ClientApp(const char* cfgFile)
{
  bool bInited = RpcClientApplication::Init(cfgFile);
  if (!bInited)
    throw std::runtime_error("failed to init rpc.");

  Family family;
  family.address = "Guangdong Zhuhai";
  family.family_name = "YY";

  Person person;
  person.name = "Adapter";
  person.sex = Male;
  for (int i = 0; i < 4; ++i)
  {
    person.person_id = i;
    person.age = 18 + i;
    family.members.push_back(person);
  }

  for (int i = 0; i < 100; ++i)
  {
    family.family_id = i;
    m_familys.push_back(family);
  }
    
  Init(cfgFile);
}

int ClientApp::Init(const char* cfgFile)
{
  m_mainThreadId = std::this_thread::get_id();

  SubscribeEventNoParam();
  SubscribeEventSimpleParam();
  SubscribeEventUserDefineTypeParam();

  CallTest();

  return 0;
}

int ClientApp::UnInit()
{
  return 0;
}

void ClientApp::CallTest()
{
  FamilyApi::Hello();

  SyncCallDemo();

  AsyncCallDemo();

  CallbackResultDemo();
}

void ClientApp::SyncCallDemo()
{
  std::vector<Family> fimilys;
  try
  {
    fimilys = FamilyApi::GetAllFamily();
  }
  catch (std::runtime_error& e)
  {
    std::cout << "FamilyApi::GetAllFamily runtime_error:" << e.what() << std::endl;
  }

  bool ret = false;
  try
  {
    ret = FamilyApi::AddFamily(m_familys[0]);
  }
  catch (std::runtime_error& e)
  {
    std::cout << "FamilyApi::AddFamily runtime_error:" << e.what() << std::endl;
  }

  std::vector<Family> new_fimilys;
  try
  {
    new_fimilys = FamilyApi::GetAllFamily();
  }
  catch (std::runtime_error& e)
  {
    std::cout << "FamilyApi::GetAllFamily runtime_error:" << e.what() << std::endl;
  }

  if (ret)
  {
    assert(new_fimilys.size() == fimilys.size() + 1);
    LOG(ERROR) << "AddFamily successed. family_id: " << m_familys[0].family_id;
  }
  else
  {
    assert(new_fimilys.size() == fimilys.size());
    LOG(ERROR) << "AddFamily failed. family_id: " << m_familys[0].family_id;
  }

  try
  {
    int32_t count = FamilyApi::GetFamilyCount();
    if (count == 1)
      count = FamilyApi::AddFamilyBatch(m_familys);
    assert(count == 99 || count == 100);
  }
  catch (std::runtime_error& e)
  {
    std::cout << "FamilyApi::AddFamilyBatch runtime_error:" << e.what() << std::endl;
    return;
  }
  
  FamilyApi::SetFamilyName(0, "ZZ");

  std::string familyName;
  try
  {
    familyName = FamilyApi::GetFamilyName(0).Wait();
    assert(familyName == "ZZ");
  }
  catch (std::runtime_error& e)
  {
    std::cout << "FamilyApi::GetAllFamily runtime_error:" << e.what() << std::endl;
    return;
  }

  FamilyApi::SetFamilyName(0, "YY");  //reset
}

void ClientApp::AsyncCallDemo()
{
  auto deferRet = FamilyApi::GetAllFamily();

  //////////////
  // do somethings
  /////////////

  try
  {
    std::vector<Family> fimilys = deferRet.Wait();
    assert(fimilys.size() == m_familys.size());
  }
  catch (std::runtime_error& e)
  {
    std::cout << "FamilyApi::GetAllFamily runtime_error:" << e.what() << std::endl;
    return;
  }

  //wait again
  std::vector<Family> fimilys = deferRet.Wait();
  assert(fimilys.size() == m_familys.size());
}

void ClientApp::CallbackResultDemo()
{
#ifndef ORPC_USE_FIBER
  FamilyApi::GetAllFamily().Then<any_thread_t>(
    [this](const std::vector<Family>& fimilys)
  {
    assert(m_mainThreadId != std::this_thread::get_id());
    assert(fimilys.size() == m_familys.size());
  },
    [](int error)
  {
    std::cout << "FamilyApi::GetAllFamily error:" << error << std::endl;
  });

  FamilyApi::GetAllFamily().Then(std::bind(&ClientApp::OnGetAllFamilysThisThread, this, std::placeholders::_1));

  FamilyApi::GetAllFamily().Then<any_thread_t>(std::bind(&ClientApp::OnGetAllFamilysAnyThread, this, std::placeholders::_1),
    std::bind(&ClientApp::OnGetAllFamilysError, this, std::placeholders::_1));
#endif  //! #ifndef ORPC_USE_FIBER
}

void ClientApp::OnGetAllFamilysThisThread(const std::vector<Family>& fimilys)
{
  assert(m_mainThreadId == std::this_thread::get_id());
  assert(fimilys.size() == m_familys.size());
}

void ClientApp::OnGetAllFamilysAnyThread(const std::vector<Family>& fimilys)
{
  assert(m_mainThreadId != std::this_thread::get_id());
  assert(fimilys.size() == m_familys.size());
}

void ClientApp::OnGetAllFamilysError(int32_t error)
{
  std::cout << "FamilyApi::GetAllFamily error:" << error << std::endl;
}

void ClientApp::SubscribeEventNoParam()
{
  FamilyEventApi::Haha.Subscribe([](void) { std::cout << "HaHa!" << std::endl; });
}

void ClientApp::SubscribeEventSimpleParam()
{
  FamilyEventApi::FamilyNameChanged.Subscribe(std::bind(&ClientApp::OnFamilyNameChanged, this, std::placeholders::_1, std::placeholders::_2));
}

void ClientApp::SubscribeEventUserDefineTypeParam()
{
  FamilyEventApi::FamilyAdded.Subscribe(std::bind(&ClientApp::OnFamilyAdded, this, std::placeholders::_1));
}

void ClientApp::OnFamilyNameChanged(int32_t family_id, const std::string& family_name)
{
  std::cout << "OnFamilyNameChanged family_id:" << family_id << " family_name:" << family_name << std::endl;
}

void ClientApp::OnFamilyAdded(const Family& new_family)
{
  std::cout << "OnFamilyAdded family_id:" << new_family.family_id << " family_name:" << new_family.family_name
    << " address:" << new_family.address << " member_size:" << new_family.members.size() << std::endl;
}
