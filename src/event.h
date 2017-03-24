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

#ifndef ORPC_EVENT_H_
#define ORPC_EVENT_H_

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

template <typename Func>
struct TRpcEvent;

struct TRpcEventName
{
  std::string	m_fullName;
  std::vector<std::string> m_vParamName;
};

template <typename R, typename ... Args>
struct TRpcEvent <R(Args...)>
{
public:
  TRpcEvent(const std::string& name, 
    const std::string& ns, 
    const char* funcStr);

  ~TRpcEvent();
public:
  template<typename ... Args2>
  CallResultWrapper<void> operator()(Args2... args) const;

  CallResultWrapper<void> operator()() const;

  template<typename ... Args2>
  BufferStream GetFireData(Args2... args) const;

  BufferStream GetFireData() const;

  template <typename T = this_thread_t>
  CallResultWrapper<bool> Subscribe(const std::function<R(Args...)>& func) const;
private:
  TRpcEventName* m_name;
};

#include "event.inl"

_END_ORPC_NAMESPACE_

#ifdef WIN32
#define ORPC_EVENT(method_name, func) \
inline std::string __namespace__##method_name() { return orpc::CurrentNamespaceName(__FUNCSIG__); }\
static const orpc::TRpcEvent<func> method_name{ #method_name, __namespace__##method_name(), #func};
#else
#define ORPC_EVENT(method_name, func) \
inline std::string __namespace__##method_name() { return orpc::CurrentNamespaceName(__PRETTY_FUNCTION__); }\
static const orpc::TRpcEvent<func> method_name{ #method_name, __namespace__##method_name(), #func};
#endif

#endif  //! #ifndef ORPC_EVENT_H_
