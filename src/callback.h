#ifndef YYRPC_CALLBACK_H_
#define YYRPC_CALLBACK_H_

#include <iostream>
#include <string>
#include <functional>
#include "msgpack.hpp"
#include "index_sequence.hpp"

class ICallback
{
public:
  virtual ~ICallback() {}
  virtual bool Invork(msgpack::unpacker& unp) = 0;
};

template <typename T>
struct TRpcCallback ;

template <typename ... Args>
struct TRpcCallback < bool(Args...) > : public ICallback
{
  TRpcCallback(const std::function<bool(Args...)>& func) : m_func(func) {}

  virtual bool Invork(msgpack::unpacker& unp) override;
private:
  std::function<bool(Args...)>	m_func;
};

template< typename Function, typename Tuple, std::size_t... index>
bool invoke_helper(Function&& f, Tuple&& tup, __boost::index_sequence<index...>)  {
  return f(std::get<index>(std::forward<Tuple>(tup))...);
}

template <std::size_t Arity, typename Function, typename Tuple>
bool invoke_with_tuple(Function&& f, Tuple&& tup)
{
  return invoke_helper(std::forward<Function>(f), std::forward<Tuple>(tup), __boost::make_index_sequence<Arity>{});
}

template<typename... Args>
bool CheckAndUnSerialization(msgpack::unpacker& unp, std::tuple<Args ...>& args);

bool CheckAndUnSerialization(msgpack::unpacker& unp);

template <typename ... Args>
bool TRpcCallback < bool(Args...) >::Invork(msgpack::unpacker& unp)
{
  if (!m_func)
    return false;

  std::tuple<Args...> t;
  if (!CheckAndUnSerialization(unp, t))
    return false;

  return invoke_with_tuple<sizeof...(Args)>(m_func, t);
}

template <>
struct TRpcCallback < bool(void) > : public ICallback
{
  TRpcCallback(const std::function<bool(void)>& func) : m_func(func) {}
  virtual bool Invork(msgpack::unpacker& unp) override
  {
    msgpack::object_handle o;
    if (unp.next(o))
      return false;
    return m_func();
  }
private:
  std::function<bool(void)>	m_func;
};

#endif  //! #ifndef YYRPC_CALLBACK_H_
