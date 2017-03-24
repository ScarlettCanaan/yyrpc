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

#ifndef __ORPC_METHOD_AUTH_H_
#define __ORPC_METHOD_AUTH_H_

#include "../build_config.h"
#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <mutex>
#include <unordered_map>

_START_ORPC_NAMESPACE_

struct CallbackInvorkParam;

class MethodAuth
{
public:
  int Init();
  int UnInit();
public:
  void OnClientConnected(int32_t appId, int64_t token, int32_t privilege);
  void OnClientDisconnected(int32_t appId);

  void OnMethodInfo(const std::string& method_name, bool nostate, int32_t privilege);

  bool CheckAuth(const std::string& method_name, const CallbackInvorkParam& param);
private:
  std::unordered_map<int32_t, int64_t> m_peerSessions;
  std::unordered_map<int32_t, int32_t> m_peerPrivileges;
  std::unordered_map<std::string, int64_t> m_methodPrivilege;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef __ORPC_METHOD_AUTH_H_
