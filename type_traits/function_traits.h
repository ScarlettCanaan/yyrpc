#ifndef YYRPC_FUNCTION_TRAITS
#define YYRPC_FUNCTION_TRAITS

#include <type_traits>

#if WIN32 && _MSC_VER <= 1800
#define constexpr const
#endif  //! #ifdef WIN32 && _MSC_VER <= 1800

//Specialize template declare (catch all case)
template<typename T>
struct function_traits;

template<typename R, typename ...Args>
struct function_traits < R(Args...) >
{
  static constexpr std::size_t nargs = sizeof...(Args);
  using return_type = R;
  typedef R function_type(Args...);
  template <std::size_t index>
  struct argument { using type = typename std::tuple_element<index, std::tuple<Args...>>::type; };
};

template<typename R, typename ...Args>
struct function_traits<R(*)(Args...)> : public function_traits < R(Args...) >
{
};

template<typename R>
struct function_traits < R(void) >
{
  static constexpr std::size_t nargs = 0;
  using return_type = R;
};

template<typename R>
struct function_traits<R(*)(void)> : public function_traits < R(void) >
{
};

#endif  //! #ifndef YYRPC_FUNCTION_TRAITS
