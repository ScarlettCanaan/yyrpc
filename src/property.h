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

#ifndef ORPC_PROPERTY_H_
#define ORPC_PROPERTY_H_

#include "build_config.h"
#include "stdint.h"
#include <iostream>
#include <list>
#include <string>
#include <assert.h>
#include <type_traits>
#include <sstream>
#include <tuple>
#include <vector>

_START_ORPC_NAMESPACE_

//json don't support string with '\0'
//std::string use for readable string, and BinaryString use for binary
struct BinaryString
{
  std::string data;
};

#ifdef WIN32
template <typename T>
class has_property_meta
{
private:
  template <typename U> static uint8_t test(decltype(U::PropertyMeta));
  template <typename U> static uint16_t test(...);
public:
  static const bool value = sizeof(test<T>(0)) == sizeof(uint8_t);
};

template<typename T> struct is_model : std::integral_constant < bool, has_property_meta<T>::value > {};

#define ORPC_PROPERTY(propertyName, ...) \
auto PropertyMeta() -> decltype(std::tie(propertyName, ##__VA_ARGS__)) { return std::tie(propertyName, ##__VA_ARGS__); }\
static const std::vector<std::string>& PropertyMetaVName()\
{\
  static const char* c_name = #propertyName#__VA_ARGS__;\
  static std::vector<std::string> names;\
  if (names.empty())\
    names = orpc::tokenize_property_name(c_name);\
  return names;\
}

#else

template <typename T>
class has_property_meta
{
private:
  template <typename U> static uint8_t test(decltype(&U::PropertyMeta));
  template <typename U> static uint16_t test(...);
public:
  static const bool value = sizeof(test<T>(0)) == sizeof(uint8_t);
};

template<typename T> struct is_model : std::integral_constant < bool, has_property_meta<T>::value > {};

#define ORPC_PROPERTY(propertyName, ...) \
auto PropertyMeta() { return std::tie(propertyName, ##__VA_ARGS__); }\
static const std::vector<std::string>& PropertyMetaVName()\
{\
  static const char* c_name = #propertyName#__VA_ARGS__;\
  static std::vector<std::string> names;\
  if (names.empty())\
    names = orpc::tokenize_property_name(c_name);\
  return names;\
}

#endif  //! #ifdef WIN32

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_PROPERTY_H_
