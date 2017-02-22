#ifndef YYRPC_METHOD_H_
#define YYRPC_METHOD_H_

#include <string>
#include <type_traits>
#include <tuple>
#include "type_traits/function_traits.h"
#include "serialization.h"
#include "call_manager.h"

template<typename Func, std::size_t I = function_traits<Func>::nargs - 1>
struct ParamIterator
{
  template<typename... Args>
  static void pack(std::stringstream& s, const std::tuple<Args ...>& args)
  {
    using define_type = typename function_traits<Func>::template argument<I>::type;
    using raw_type = typename std::remove_cv<typename std::remove_reference<define_type>::type>::type;
    using param_type = typename std::tuple_element<I, std::tuple<Args ...>>::type;
    static_assert(std::is_convertible<param_type, define_type>::value, "RPC_CALL_ERROR: argument type mismatch.");
    serialization<raw_type>(s, std::get<I>(args));
    ParamIterator<Func, I - 1>::pack(s, args);
  }
};

template<typename Func>
struct ParamIterator < Func, 0 >
{
  template<typename... Args>
  static void pack(std::stringstream& s, const std::tuple<Args ...>& args)
  {
    using define_type = typename function_traits<Func>::template argument<0>::type;
    using raw_type = typename std::remove_cv<typename std::remove_reference<define_type>::type>::type;
    using param_type = typename std::tuple_element<0, std::tuple<Args ...>>::type;
    static_assert(std::is_convertible<param_type, define_type>::value, "RPC_CALL_ERROR: argument type mismatch.");
    serialization<raw_type>(s, std::get<0>(args));
  }
};

template<typename Func, typename... Args>
void CheckAndSerialization(const std::string& name, std::stringstream& s, Args ...args)
{
  using traits = function_traits < Func > ;
  static_assert(sizeof...(args) == traits::nargs, "RPC_CALL_ERROR: argument count mismatch.");
  ParamIterator<Func>::pack(s, std::make_tuple(args...));
}

template<typename Func>
void CheckAndSerialization(const std::string& name, std::stringstream& s)
{
  using traits = function_traits < Func > ;
  static_assert(traits::nargs == 0, "RPC_CALL_ERROR: argument count mismatch.");
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
  explicit TRpcMethod(const std::string& name, const std::string& ns) 
  { fullname_ = ns + name; }

  template<typename ... Args>
  std::shared_ptr<AsyncResult<R>> operator()(Args... args) const
  {
    std::stringstream s;
    CheckAndSerialization<R(Args...)>(fullname_, s, args...);
    return AsyncCallManager::GetInstance().Call<R>(0, s);
  }
private:
  std::string	fullname_;
};

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
