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

#ifndef ORPC_FUNCTION_TRAITS
#define ORPC_FUNCTION_TRAITS

#include <type_traits>

#if WIN32 && _MSC_VER <= 1800
#define constexpr const
#endif  //! #ifdef WIN32 && _MSC_VER <= 1800

_START_ORPC_NAMESPACE_

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

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_FUNCTION_TRAITS
