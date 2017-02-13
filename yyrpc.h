#ifndef YY_RPC_H_
#define YY_RPC_H_

#include <type_traits>
#include <tuple>
#include "type_traits/function_traits.h"
#include "serialization.h"

template<typename Func, std::size_t I = function_traits<Func>::nargs - 1>
struct ParamPacker
{
  template<typename... Args>
  static void pack(std::stringstream& s, const std::tuple<Args ...>& args)
  {
    using define_type = typename function_traits<Func>::template argument<I>::type;
    using raw_type = typename std::remove_cv<typename std::remove_reference<define_type>::type>::type;
    using param_type = typename std::tuple_element<I, std::tuple<Args ...>>::type;
    static_assert(std::is_convertible<param_type, define_type>::value, "RPC_CALL_ERROR: argument type mismatch.");
    serialization<raw_type>(s, std::get<I>(args));
    ParamPacker<Func, I - 1>::pack(s, args);
  }
};

template<typename Func>
struct ParamPacker <Func, 0>
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
void RpcFunctionTraits(Args ...args)
{
  using traits = function_traits<Func>;
  static_assert(sizeof...(args) == traits::nargs, "RPC_CALL_ERROR: argument count mismatch.");
  std::stringstream s;
  ParamPacker<Func>::pack(s, std::make_tuple(args...));
}

template<typename Func>
void RpcFunctionTraits()
{
  using traits = function_traits<Func>;
  static_assert(traits::nargs == 0, "RPC_CALL_ERROR: argument count mismatch.");
  std::stringstream s;
}

template<typename Func, typename... Args>
void SyncCall(const char* method_name, Args ...args)
{
  RpcFunctionTraits<Func>(args...);
  return;
}

template<typename Func>
void SyncCall(const char* method_name)
{
  RpcFunctionTraits<Func>();
  return;
}

#define SYNC_CALL(methed_name, ...) \
  SyncCall<decltype(methed_name)>(#methed_name, ##__VA_ARGS__)

#endif  //! #ifndef YY_RPC_H_
