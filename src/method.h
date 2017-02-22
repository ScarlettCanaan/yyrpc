#ifndef YYRPC_METHOD_H_
#define YYRPC_METHOD_H_

#include <string>
#include <type_traits>
#include <tuple>
#include "type_traits/function_traits.h"
#include "serialization.h"
#include "unserialization.h"
#include "callback.h"
#include "caller_manager.h"
#include "callee_manager.h"
#include "endpoint/endpoint_manager.h"
#include "id_generator.h"

class EndPoint;

template<typename Func, std::size_t I = function_traits<Func>::nargs - 1>
struct ParamIterator
{
  using define_type = typename function_traits<Func>::template argument<I>::type;
  using raw_type = typename std::remove_cv<typename std::remove_reference<define_type>::type>::type;

  template<typename... Args>
  static bool pack(std::stringstream& s, const std::tuple<Args ...>& args);

  template<typename... Args>
  static bool unpack(msgpack::unpacker& unp, std::tuple<Args ...>& args);
};

template<typename Func, std::size_t I>
  template<typename... Args>
bool ParamIterator<Func, I>::pack(std::stringstream& s, const std::tuple<Args ...>& args)
{
  using param_type = typename std::tuple_element<I, std::tuple<Args ...>>::type;
  static_assert(std::is_convertible<param_type, define_type>::value, "RPC_CALL_ERROR: argument type mismatch.");
  bool r = serialization<raw_type>(s, std::get<I>(args));
  if (!r)
    return false;
  return ParamIterator<Func, I - 1>::pack(s, args);
}

template<typename Func, std::size_t I>
template<typename... Args>
bool ParamIterator<Func, I>::unpack(msgpack::unpacker& unp, std::tuple<Args ...>& args)
{
  using param_type = typename std::tuple_element<I, std::tuple<Args ...>>::type;
  static_assert(std::is_convertible<param_type, define_type>::value, "RPC_CALL_ERROR: argument type mismatch.");
  bool r = unserialization<raw_type>(unp, std::get<I>(args));
  if (!r)
    return false;
  return ParamIterator<Func, I - 1>::unpack(unp, args);
}

template<typename Func>
struct ParamIterator < Func, 0 >
{
  using define_type = typename function_traits<Func>::template argument<0>::type;
  using raw_type = typename std::remove_cv<typename std::remove_reference<define_type>::type>::type;

  template<typename... Args>
  static bool pack(std::stringstream& s, const std::tuple<Args ...>& args);

  template<typename... Args>
  static bool unpack(msgpack::unpacker& unp, std::tuple<Args ...>& args);
};

template<typename Func> 
template<typename... Args>
bool ParamIterator < Func, 0 >::pack(std::stringstream& s, const std::tuple<Args ...>& args)
{
  using param_type = typename std::tuple_element<0, std::tuple<Args ...>>::type;
  static_assert(std::is_convertible<param_type, define_type>::value, "RPC_CALL_ERROR: argument type mismatch.");
  return serialization<raw_type>(s, std::get<0>(args));
}

template<typename Func>
template<typename... Args>
bool ParamIterator < Func, 0 >::unpack(msgpack::unpacker& unp, std::tuple<Args ...>& args)
{
  using param_type = typename std::tuple_element<0, std::tuple<Args ...>>::type;
  static_assert(std::is_convertible<param_type, define_type>::value, "RPC_CALL_ERROR: argument type mismatch.");
  return unserialization<raw_type>(unp, std::get<0>(args));
}

template<typename Func, typename... Args>
bool CheckAndSerialization(const std::string& name, std::stringstream& s, Args ...args)
{
  using traits = function_traits < Func > ;
  static_assert(sizeof...(args) == traits::nargs, "RPC_CALL_ERROR: argument count mismatch.");
  return ParamIterator<Func>::pack(s, std::make_tuple(args...));
}

template<typename Func>
bool CheckAndSerialization(const std::string& name, std::stringstream& s)
{
  using traits = function_traits < Func > ;
  static_assert(traits::nargs == 0, "RPC_CALL_ERROR: argument count mismatch.");
  return true;
}

template<typename... Args>
bool CheckAndUnSerialization(msgpack::unpacker& unp, std::tuple<Args ...>& args)
{
  return ParamIterator<void(Args...)>::unpack(unp, args);
}

inline bool CheckAndUnSerialization(msgpack::unpacker& unp)
{
  return true;
}

template <typename Func>
struct TRpcMethod;

inline std::string CurrentNamespaceName(const std::string& prettyFunction)
{
  size_t colonsA = prettyFunction.find("__namespace__");
  size_t colonsB = prettyFunction.rfind(" ", colonsA);
  if (colonsA <= colonsB)
    return "";

  return prettyFunction.substr(colonsB, colonsA - colonsB);
}

template <typename R, typename ... Args>
struct TRpcMethod < R(Args...) >
{
  TRpcMethod(const std::string& name, const std::string& ns) 
  { fullname_ = ns + name; }

  template<typename ... Args2>
  std::shared_ptr<AsyncResult<R>> operator()(Args2... args) const;

  template<typename ... Args2>
  std::shared_ptr<AsyncResult<R>> operator()(const std::shared_ptr<EndPoint>& endpoint, Args2... args) const;

  std::shared_ptr<AsyncResult<R>> operator()() const;

  bool bind(std::function<bool(Args...)> func) const;
private:
  std::string	fullname_;
};

template <typename R, typename ... Args>
template<typename ... Args2>
std::shared_ptr<AsyncResult<R>> TRpcMethod < R(Args...) >::operator()(Args2... args) const
{
  std::stringstream s;
  CheckAndSerialization<R(Args...)>(fullname_, s, args...);
  auto endpoint = EndPointManager::GetInstance().QueryEndPoint(fullname_);
  uint32_t session_id = get_sessionid();
  return CallerManager::GetInstance().Call<R>(endpoint, session_id, s);
}

template <typename R, typename ... Args>
template<typename ... Args2>
std::shared_ptr<AsyncResult<R>> TRpcMethod < R(Args...) >::operator()(const std::shared_ptr<EndPoint>& endpoint, Args2... args) const
{
  std::stringstream s;
  CheckAndSerialization<R(Args...)>(fullname_, s, args...);
  uint32_t session_id = get_sessionid();
  return CallerManager::GetInstance().Call<R>(endpoint, session_id, s);
}

template <typename R, typename ... Args>
std::shared_ptr<AsyncResult<R>> TRpcMethod < R(Args...) >::operator()() const
{
  std::stringstream s;
  CheckAndSerialization<R(void)>(fullname_, s);
  auto endpoint = EndPointManager::GetInstance().QueryEndPoint(fullname_);
  uint32_t session_id = get_sessionid();
  return CallerManager::GetInstance().Call<R>(endpoint, session_id, s);
}

template <typename R, typename ... Args>
bool TRpcMethod < R(Args...) >::bind(std::function<bool(Args...)> func) const
{
  return CalleeManager::GetInstance().BindApi(fullname_, func);
}

#ifdef WIN32
#define YYRPC_METHOD(method_name, ...) \
inline std::string __namespace__##method_name() { return CurrentNamespaceName(__FUNCSIG__); }\
static const TRpcMethod<__VA_ARGS__> method_name{ #method_name, __namespace__##method_name()};
#else
#define YYRPC_METHOD(method_name, ...) \
inline std::string __namespace__##method_name() { return CurrentNamespaceName(__PRETTY_FUNCTION__); }\
static const TRpcMethod<__VA_ARGS__> method_name{ #method_name, __namespace__##method_name()};
#endif

#endif  //! #ifndef YYRPC_METHOD_H_
