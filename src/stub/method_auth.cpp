/*
* The MIT License (MIT)
*
* Copyright (c) 2017-2018 youjing@yy.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "method_auth.h"
#include "../acceptor/rpc_client_accept.h"
#include "../util/base_util.h"
#include "../transport/rpc_tcp_server_transport.h"
#include "callee_manager.h"

_START_ORPC_NAMESPACE_

int MethodAuth::Init()
{
  std::function<void(int32_t, int64_t, int32_t)> funcConnected =
    std::bind(&MethodAuth::OnClientConnected, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  CalleeManager::GetInstance().BindApi("NameCenterAPi::OnClientConnect", funcConnected, true);

  std::function<void(int32_t)> funcDisconnected =
    std::bind(&MethodAuth::OnClientDisconnected, this, std::placeholders::_1);
  CalleeManager::GetInstance().BindApi<void, int32_t>("NameCenterAPi::OnClientDisconnected", funcDisconnected, true);

  return 0;
}

int MethodAuth::UnInit()
{
  return 0;
}

bool MethodAuth::CheckAuth(const std::string& method_name, const CallbackInvorkParam& param)
{
  if (m_methodPrivilege.empty())
    return true;

  auto it = m_methodPrivilege.find(method_name);
  if (it == m_methodPrivilege.end())
    return true;

  int32_t appId = param.transport->GetPeerId();
  int32_t privilege = param.transport->GetPeerPrivilege();
  int64_t session = param.transport->GetPeerSessionToken();

  auto itSession = m_peerSessions.find(appId);
  if (itSession == m_peerSessions.end())
    return false;

  auto itPrivilege = m_peerPrivileges.find(appId);
  if (itPrivilege == m_peerPrivileges.end())
    return false;

  if (privilege < itPrivilege->second || session != itSession->second)
    return false;

  return true;
}

void MethodAuth::OnClientConnected(int32_t appId, int64_t token, int32_t privilege)
{
  m_peerSessions[appId] = token;
  m_peerPrivileges[appId] = privilege;
  ORPC_LOG(ERROR) << "OnClientConnect appId:" << appId << " token : " << token << " privilege : " << privilege;
}

void MethodAuth::OnClientDisconnected(int32_t appId)
{
  m_peerSessions.erase(appId);
  m_peerPrivileges.erase(appId);
  ORPC_LOG(ERROR) << "OnClientDisconnected appId:" << appId;
}

void MethodAuth::OnMethodInfo(const std::string& method_name, bool nostate, int32_t privilege)
{
  m_methodPrivilege[method_name] = privilege;
  ORPC_LOG(ERROR) << "RegisterMethod method_name:" << method_name << " privilege: " << privilege;
}

_END_ORPC_NAMESPACE_
