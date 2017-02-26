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

#endif

#define YYRPC_PROPERTY(...) auto meta() -> decltype(std::tie(__VA_ARGS__)) { return std::tie(__VA_ARGS__); } \

#endif  //! #ifndef YYRPC_PROPERTY_H_
