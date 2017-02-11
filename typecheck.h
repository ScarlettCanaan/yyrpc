#ifndef TYPE_CHECK_H_
#define TYPE_CHECK_H_

#include <tuple>
#include <functional>
#include <type_traits>

#if WIN32 && _MSC_VER <= 1800
#define constexpr const
#endif  //! #ifdef WIN32 && _MSC_VER <= 1800

//Specialize template declare (catch all case)
template<typename T>
struct function_traits;

template<typename R, typename ...Args>
struct function_traits<R(Args...)>
{
  static constexpr std::size_t nargs = sizeof...(Args);
  using return_type = R;
  template <std::size_t index>
  struct argument { using type = typename std::tuple_element<index, std::tuple<Args...>>::type; };
};

template<typename R, typename ...Args>
struct function_traits<R(*)(Args...) > : public function_traits<R(Args...)>
{
};

template<typename R>
struct function_traits <R(void)>
{
  static constexpr std::size_t nargs = 0;
  using return_type = R;
};

template<typename R>
struct function_traits<R(*)(void)> : public function_traits<R(void)> 
{
};

template<typename Func, std::size_t I = function_traits<Func>::nargs - 1>
struct ParamPacker
{
  template <typename... Args>
  static void pack(Args ...args) 
  {
    using traits = function_traits<Func>;
    using dst_param_type = typename traits::template argument<I>::type;
    using src_param_type = typename std::tuple_element<I, decltype(std::make_tuple(args...))>::type;
    static_assert(std::is_same<dst_param_type, src_param_type>::value, "RPC_CALL_ERROR: argument type mismatch.");
    ParamPacker<Func, I - 1>::pack(args...);
  }
};

template<typename Func>
struct ParamPacker<Func, 0>
{
  template <typename... Args>
  static void pack(Args ...args)
  {
    using traits = function_traits<Func>;
    using func_traits_type = typename traits::template argument<0>::type;
    using arg_traits_type = typename std::tuple_element<0, decltype(std::make_tuple(args...))>::type;
    static_assert(std::is_same<func_traits_type, arg_traits_type>::value, "RPC_CALL_ERROR: argument type mismatch.");
  }
};

template<typename Func, typename... Args>
void RpcFunctionTraitsCheck(Args ...args) 
{
  using traits = function_traits<Func>;
  static_assert(sizeof...(args) == traits::nargs, "RPC_CALL_ERROR: argument count mismatch.");
  ParamPacker<Func>::pack(args...);
}

template<typename Func>
void RpcFunctionTraitsCheck()
{
  using traits = function_traits<Func>;
  static_assert(traits::nargs == 0, "RPC_CALL_ERROR: argument count mismatch.");
}

template<typename Func, typename... Args>
void typeCheck(const char* method_name, Args ...args) 
{
  RpcFunctionTraitsCheck<Func>(args...);

  return;
}

template<typename Func>
void typeCheck(const char* method_name)
{
  RpcFunctionTraitsCheck<Func>();
  return;
}

#define TYPE_CHECK(method_name, ...) \
  typeCheck<decltype(method_name)>(#method_name, ##__VA_ARGS__)

#endif  //! #ifndef TYPE_CHECK_H_ 
