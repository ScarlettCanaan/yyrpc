#include <iostream>
#include <string>
#include "../proto/test_api.h"
#include "server.h"
#include "orpc.h"
#include <assert.h>
#include "glog/logging.h"

using namespace orpc;

ServerApp::ServerApp(const char* cfgFile)
{
  bool bInited = RpcServerApplication::Init(cfgFile);
  if (!bInited)
    throw std::runtime_error("failed to init rpc.");

  Init(cfgFile);
}

int ServerApp::Init(const char* cfgFile)
{
  m_mainThreadId = std::this_thread::get_id();

  BindNoReturnNoParam();

  BindNoReturnWithParam();
  BindNoParamWithReturn();
  BindSimpleReturnOneParam();
  BindStlContainerReturnOrParam();
  BindUserDefineTypeReturnOrParam();

  return 0;
}

int ServerApp::UnInit()
{
  return 0;
}

// test support lambda and TcpPeer*
void ServerApp::BindNoReturnNoParam()
{
  FamilyApi::Hello.BindWithPeer<any_thread_t>([this](TcpPeer* peer)
  {
    assert(m_mainThreadId != std::this_thread::get_id());
    std::cout << "Hello from: " << peer->GetPeerIp() << " port: " << peer->GetPeerPort() << std::endl;
    FireEventNoParam();
  });
}

void ServerApp::BindNoReturnWithParam()
{
  FamilyApi::SetFamilyName.Bind(
    std::bind(&ServerApp::OnSetFamilyName, this, std::placeholders::_1, std::placeholders::_2));
}

void ServerApp::OnSetFamilyName(int32_t family_id, const std::string& family_name)
{
  assert(m_mainThreadId == std::this_thread::get_id());
  auto it = m_familys.find(family_id);
  if (it == m_familys.end())
  {
    LOG(ERROR) << "OnSetFamilyName failed, cant find family_id: " << family_id;
    return;
  }

  it->second.family_name = family_name;
  FireEventSimpleParam(family_id, family_name);
}

void ServerApp::BindNoParamWithReturn()
{
  FamilyApi::GetFamilyCount.Bind<any_thread_t>(std::bind(&ServerApp::OnGetFamilyCount, this));
}

int32_t ServerApp::OnGetFamilyCount(void)
{
  return m_familys.size();
}

void ServerApp::BindSimpleReturnOneParam()
{
  FamilyApi::GetFamilyName.Bind(std::bind(&ServerApp::OnGetFamilyName, this, std::placeholders::_1));
}

std::string ServerApp::OnGetFamilyName(int32_t family_id)
{
  assert(m_mainThreadId == std::this_thread::get_id());
  auto it = m_familys.find(family_id);
  if (it == m_familys.end())
  {
    LOG(ERROR) << "OnGetFamilyName failed, cant find family_id: " << family_id;
    return "Unknown";
  }

  return it->second.family_name;
}

void ServerApp::BindStlContainerReturnOrParam()
{
  FamilyApi::AddFamilyBatch.Bind(std::bind(&ServerApp::OnAddFamilyBatch, this, std::placeholders::_1));
  FamilyApi::GetAllFamily.Bind(std::bind(&ServerApp::OnGetAllFamily, this));
}

void ServerApp::BindUserDefineTypeReturnOrParam()
{
  FamilyApi::AddFamily.Bind(std::bind(&ServerApp::OnAddFamily, this, std::placeholders::_1));
  FamilyApi::GetFamily.Bind(std::bind(&ServerApp::OnGetFamily, this, std::placeholders::_1));
}

int32_t ServerApp::OnAddFamilyBatch(const std::vector<Family>& familys)
{
  int32_t count = 0;
  for (auto it : familys)
  {
    if (OnAddFamily(it))
      ++count;
  }
  return count;
}

bool ServerApp::OnAddFamily(const Family& family)
{
  auto it = m_familys.find(family.family_id);
  if (it != m_familys.end())
    return false;

  FireEventUserDefineTypeParam(family);
  m_familys[family.family_id] = family;
  return true;
}

Family ServerApp::OnGetFamily(int32_t family_id)
{
  static Family s_family;
  auto it = m_familys.find(family_id);
  if (it == m_familys.end())
  {
    LOG(ERROR) << "OnGetFamily failed, cant find family_id: " << family_id;
    s_family.family_id = -1;
    return s_family;
  }

  return it->second;
}

std::vector<Family> ServerApp::OnGetAllFamily(void)
{
  std::vector<Family> familys;
  for (auto it : m_familys)
    familys.push_back(it.second);
  return familys;
}

void ServerApp::FireEventNoParam()
{
  FamilyEventApi::Haha();
}

void ServerApp::FireEventSimpleParam(int32_t family_id, const std::string& family_name)
{
  FamilyEventApi::FamilyNameChanged(family_id, family_name);
}

void ServerApp::FireEventUserDefineTypeParam(const Family& new_family)
{
  FamilyEventApi::FamilyAdded(new_family);
}
