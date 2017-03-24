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

#ifndef ORPC_CALLBACK_H_
#define ORPC_CALLBACK_H_

//callback is the method implement for service, or the event responser

#include "../build_config.h"
#include <iostream>
#include <string>
#include <functional>
#include "../index_sequence.hpp"
#include <thread>
#include <memory>
#include "../transport/rpc_tcp_server_transport.h"
#include "../marshal/serialization.h"
#include "../marshal/unserialization.h"
#include "../error_def.h"
#include "../buff_stream.h"

_START_ORPC_NAMESPACE_

struct CallbackInvorkParam
{
public:
  CallbackInvorkParam() {}
  CallbackInvorkParam(const std::shared_ptr<msgpack::object_handle>& obj_, int64_t session_id_, const std::shared_ptr<RpcTcpServerTransport>& transport_)
    : obj(obj_), session_id(session_id_), transport(transport_) { }
  CallbackInvorkParam(const std::shared_ptr<msgpack::object_handle>& obj_)
    : obj(obj_) { }

public:
  std::shared_ptr<msgpack::object_handle> obj;  //call param obj
  int64_t session_id;
  std::shared_ptr<RpcTcpServerTransport> transport;
};

class ICallback
{
public:
  virtual ~ICallback() {}
  virtual bool Invork(const CallbackInvorkParam& param) const = 0;
};

class TcpPeer;

template <typename T>
class TRpcCallback;

//with param, has return, without peer
template <typename R, typename ... Args>
class TRpcCallback < R(Args...) > : public ICallback
{
public:
  TRpcCallback(const std::function<R(Args...)>& func)
  : m_func(func) {}

  virtual bool Invork(const CallbackInvorkParam& param) const override;
private:
  std::function<R(Args...)>	m_func;
};

//with param, has return, with peer
template <typename R, typename ... Args>
class TRpcCallback < R(TcpPeer*, Args...) > : public ICallback
{
public:
  TRpcCallback(const std::function<R(TcpPeer*, Args...)>& func)
  : m_func(func) {}

  virtual bool Invork(const CallbackInvorkParam& param) const override;
private:
  std::function<R(TcpPeer*, Args...)>	m_func;
};

//with param, no return, without peer
template <typename ... Args>
class TRpcCallback < void(Args...) > : public ICallback
{
public:
  TRpcCallback(const std::function<void(Args...)>& func)
  : m_func(func) {}

  virtual bool Invork(const CallbackInvorkParam& param) const override;
private:
  std::function<void(Args...)>	m_func;
};

//with param, no return, with peer
template <typename ... Args>
class TRpcCallback < void(TcpPeer*, Args...) > : public ICallback
{
public:
  TRpcCallback(const std::function<void(TcpPeer*, Args...)>& func)
  : m_func(func) {}

  virtual bool Invork(const CallbackInvorkParam& param) const override;
private:
  std::function<void(TcpPeer*, Args...)>	m_func;
};

//no param, has return, without peer
template <typename R>
class TRpcCallback: public ICallback
{
public:
  TRpcCallback(const std::function<R(void)>& func)
  : m_func(func) {}

  virtual bool Invork(const CallbackInvorkParam& param) const override;
private:
  std::function<R(void)>	m_func;
};

//no param, has return, with peer
template <typename R>
class TRpcCallback < R(TcpPeer*) > : public ICallback
{
public:
  TRpcCallback(const std::function<R(TcpPeer*)>& func)
    : m_func(func) {}

  virtual bool Invork(const CallbackInvorkParam& param) const override;
private:
  std::function<R(TcpPeer*)>	m_func;
};

//no param, no return, without peer
template <>
class TRpcCallback <void> : public ICallback
{
public:
  TRpcCallback(const std::function<void(void)>& func)
  : m_func(func) {}

  virtual bool Invork(const CallbackInvorkParam& param) const override;
private:
  std::function<void(void)>	m_func;
};

inline bool TRpcCallback <void>::Invork(const CallbackInvorkParam& param) const
{
  m_func();
  param.transport->SendError(param.session_id, ORPC_ERROR_SUCESS);
  return true;
}

//no param, no return, with peer
template <>
class TRpcCallback <void(TcpPeer*)> : public ICallback
{
public:
  TRpcCallback(const std::function<void(TcpPeer*)>& func)
    : m_func(func) {}

  virtual bool Invork(const CallbackInvorkParam& param) const override;
private:
  std::function<void(TcpPeer*)>	m_func;
};

inline bool TRpcCallback <void(TcpPeer*)>::Invork(const CallbackInvorkParam& param) const
{
  m_func(param.transport->GetPeer().get());
  param.transport->SendError(param.session_id, ORPC_ERROR_SUCESS);
  return true;
}

class CallbackWrapper
{
public:
  CallbackWrapper()  { }
  CallbackWrapper(const std::shared_ptr < ICallback >& cb, std::thread::id id)
    : m_callback(cb), m_callThreadId(id) {}
public:
  bool Invork(const CallbackInvorkParam& param) const;

  operator bool() const { return m_callback.get() != 0; }
  std::thread::id GetCallThreadId() const { return m_callThreadId; }
private:
  std::shared_ptr <ICallback> m_callback;
  std::thread::id m_callThreadId;
};

#include "callback.inl"

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_CALLBACK_H_
