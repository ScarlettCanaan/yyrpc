#ifndef YYRPC_PROPERTY_H_
#define YYRPC_PROPERTY_H_

#include "stdint.h"
#include <iostream>
#include <list>
#include <string>
#include <assert.h>
#include <type_traits>
#include <sstream>
#include <tuple>

#ifdef WIN32

template <typename T>
class has_meta
{
private:
  template <typename U> static uint8_t test(decltype(U::meta));
  template <typename U> static uint16_t test(...);
public:
  static const bool value = sizeof(test<T>(0)) == sizeof(uint8_t);
};

template<typename T> struct is_model : std::integral_constant < bool, has_meta<T>::value > {};

template<typename T>
static inline auto apply(T const & args)->decltype(args)
{
  return args;
}

template<typename T, typename T1, typename... Args>
static inline auto apply(T const & t, const T1& first, const Args&... args) ->decltype(apply(std::tuple_cat(t, std::make_tuple((typename std::remove_cv<T1>::type)first)), args...))
{
  return apply(std::tuple_cat(t, std::make_tuple((typename std::remove_cv<T1>::type)first)), args...);
}

#define YYRPC_PROPERTY(...) auto meta() -> decltype(apply(std::tuple<>(), __VA_ARGS__)) { return apply(std::tuple<>(), __VA_ARGS__); } \

#else

template <typename T>
class has_meta
{
private:
  template <typename U> static uint8_t test(decltype(&U::meta));
  template <typename U> static uint16_t test(...);
public:
  static const bool value = sizeof(test<T>(0)) == sizeof(uint8_t);
};

template<typename T> struct is_model : std::integral_constant < bool, has_meta<T>::value > {};

template<typename T>
static inline auto apply(T const & args)
{
  return args;
}

template<typename T, typename T1, typename... Args>
static inline auto apply(T const & t, const T1& first, const Args&... args)
{
  return apply(std::tuple_cat(t, std::make_tuple((typename std::remove_cv<T1>::type)first)), args...);
}

#define YYRPC_PROPERTY(...) auto meta() { return apply(std::tuple<>(), __VA_ARGS__); } \

#endif

#endif  //! #ifndef YYRPC_PROPERTY_H_
