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

#ifndef ORPC_METHOD_H_
#define ORPC_METHOD_H_

#include "build_config.h"
#include <string>
#include <type_traits>
#include <tuple>
#include "marshal/serialization.h"
#include "marshal/unserialization.h"
#include "stub/callback.h"
#include "stub/caller_manager.h"
#include "stub/callee_manager.h"
#include "endpoint/endpoint_manager.h"
#include "id_generator.h"
#include "endpoint/endpoint.h"

_START_ORPC_NAMESPACE_

class TcpPeer;

template <typename Func>
struct TRpcMethod;

struct TRpcMethodName
{
  std::string	m_fullName;
  std::vector<std::string> m_vParamName;
};

template <typename R, typename ... Args>
struct TRpcMethod <R(Args...)>
{
public:
  TRpcMethod(const std::string& name, 
    const std::string& ns, 
    const char* funcStr);

  ~TRpcMethod();
public:
  template<typename ... Args2>
  CallResultWrapper<R> operator()(Args2... args) const;

  template<typename ... Args2>
  CallResultWrapper<R> operator()(const EndPointWrapper& wrapper, Args2... args) const;

  CallResultWrapper<R> operator()() const;
  CallResultWrapper<R> operator()(const EndPointWrapper& wrapper) const;

  template <typename T = this_thread_t>
  bool Bind(const std::function<R(Args...)>& func) const;

  template <typename T = this_thread_t>
  bool BindWithPeer(const std::function<R(TcpPeer*, Args...)>& func) const;
private:
  TRpcMethodName* m_name;
};

#include "method.inl"

#ifdef WIN32
#define ORPC_METHOD(method_name, func) \
inline std::string __namespace__##method_name() { return orpc::CurrentNamespaceName(__FUNCSIG__); }\
static const orpc::TRpcMethod<func> method_name{ #method_name, __namespace__##method_name(), #func};
#else
#define ORPC_METHOD(method_name, func) \
inline std::string __namespace__##method_name() { return orpc::CurrentNamespaceName(__PRETTY_FUNCTION__); }\
static const orpc::TRpcMethod<func> method_name{ #method_name, __namespace__##method_name(), #func};
#endif

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_METHOD_H_
