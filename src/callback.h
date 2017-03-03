#ifndef YYRPC_CALLBACK_H_
#define YYRPC_CALLBACK_H_

#include <iostream>
#include <string>
#include <functional>
#include "msgpack.hpp"
#include "index_sequence.hpp"
#include <thread>
#include <memory>
#include "transport/rpc_tcp_server_transport.h"
#include "serialization.h"

class ICallback
{
public:
  virtual ~ICallback() {}
  virtual bool Invork(msgpack::unpacker& unp, 
    int64_t session_id, 
    const std::string& method_name_, 
    const std::shared_ptr<RpcTcpServerTransport>& transport) const = 0;
};

struct CallbackWrapper
{
public:
  CallbackWrapper()  {}
  CallbackWrapper(const std::shared_ptr < ICallback >& cb, std::thread::id id) 
    : callback(cb), call_thread_id(id) {}
public:
  bool Invork(msgpack::unpacker& unp,
    int64_t session_id,
    const std::string& method_name_,
    const std::shared_ptr<RpcTcpServerTransport>& transport) const;

  operator bool() const { return callback.get() != 0; }
  std::thread::id get_call_thread_id() const { return call_thread_id; }
private:
  std::shared_ptr < ICallback > callback;
  std::thread::id call_thread_id;
};

inline bool CallbackWrapper::Invork(msgpack::unpacker& unp,
  int64_t session_id,
  const std::string& method_name_,
  const std::shared_ptr<RpcTcpServerTransport>& transport) const
{
  if (!callback)
    return false;
  return callback->Invork(unp, session_id, method_name_, transport);
}

template <typename T>
struct TRpcCallback;

template <typename R, typename ... Args>
struct TRpcCallback < R(Args...) > : public ICallback
{
  TRpcCallback(const std::function<R(Args...)>& func)
  : m_func(func) {}

  virtual bool Invork(msgpack::unpacker& unp,
    int64_t session_id,
    const std::string& method_name_,
    const std::shared_ptr<RpcTcpServerTransport>& transport) const override;
private:
  std::function<R(Args...)>	m_func;
};

template<typename R, typename Function, typename Tuple, std::size_t... index>
R invoke_helper(Function&& f, Tuple&& tup, __boost::index_sequence<index...>)  {
  return f(std::get<index>(std::forward<Tuple>(tup))...);
}

template <typename R, std::size_t Arity, typename Function, typename Tuple>
R invoke_with_tuple(Function&& f, Tuple&& tup)
{
  return invoke_helper<R>(std::forward<Function>(f), std::forward<Tuple>(tup), __boost::make_index_sequence<Arity>{});
}

template<typename... Args>
bool CheckAndUnSerialization(msgpack::unpacker& unp, std::tuple<Args ...>& args);

bool CheckAndUnSerialization(msgpack::unpacker& unp);

template <typename R, typename ... Args>
bool TRpcCallback < R(Args...) >::Invork(msgpack::unpacker& unp, 
  int64_t session_id, 
  const std::string& method_name_,
  const std::shared_ptr<RpcTcpServerTransport>& transport) const
{
  if (!m_func)
    return false;

  std::tuple<Args...> t;
  if (!CheckAndUnSerialization(unp, t))
    return false;
  R r = invoke_with_tuple<R, sizeof...(Args)>(m_func, t);

  std::stringstream s;
  if (!serialization_result_header(s, "/method_result", session_id, 0))
    return false;
  if (!serialization(s, r))
    return false;
  transport->SendResult(s);
  return true;
}

template <typename R>
struct TRpcCallback < R(void) > : public ICallback
{
  TRpcCallback(const std::function<R(void)>& func)
  : m_func(func) {}

  virtual bool Invork(msgpack::unpacker& unp,
    int64_t session_id,
    const std::string& method_name_,
    const std::shared_ptr<RpcTcpServerTransport>& transport) const override;
private:
  std::function<R(void)>	m_func;
};

template <typename R>
bool TRpcCallback < R(void) >::Invork(msgpack::unpacker& unp,
  int64_t session_id,
  const std::string& method_name_,
  const std::shared_ptr<RpcTcpServerTransport>& transport) const
{
  R r = m_func();

  std::stringstream s;
  if (!serialization_result_header(s, "/method_result", session_id, 0))
    return false;
  if (!serialization(s, r))
    return false;
  transport->SendResult(s);
  return true;
}

template <>
struct TRpcCallback < void(void) > : public ICallback
{
  TRpcCallback(const std::function<void(void)>& func)
  : m_func(func) {}

  virtual bool Invork(msgpack::unpacker& unp,
    int64_t session_id,
    const std::string& method_name_,
    const std::shared_ptr<RpcTcpServerTransport>& transport) const override;
private:
  std::function<void(void)>	m_func;
};

inline bool TRpcCallback<void(void)>::Invork(msgpack::unpacker& unp,
  int64_t session_id,
  const std::string& method_name_,
  const std::shared_ptr<RpcTcpServerTransport>& transport) const
{
  m_func();
  return true;
}

#endif  //! #ifndef YYRPC_CALLBACK_H_
