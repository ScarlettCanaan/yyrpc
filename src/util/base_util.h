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

#ifndef _ORPC_BASE_UTIL_H_
#define _ORPC_BASE_UTIL_H_

#include "../build_config.h"
#include <assert.h>
#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <sstream>

#define UV_CHECK_RET_0(method, r) \
if (r < 0) {\
  ORPC_LOG(ERROR) << #method << " error: " << uv_err_name(r);\
  return;\
}

#define UV_CHECK_RET_1(method, r) \
if (r < 0) {\
  ORPC_LOG(ERROR) << #method << " error: " << uv_err_name(r);\
  return -1;\
}

#define UV_CHECK(method, r) \
if (r < 0) {\
  ORPC_LOG(ERROR) << #method << " error: " << uv_err_name(r);\
}

template <typename T>
std::string NumberToString(T Number)
{
  std::ostringstream ss;
  ss << Number;
  return ss.str();
}

template <typename T>
T StringToNumber(const std::string &Text)
{
  std::istringstream ss(Text);
  T result;
  return ss >> result ? result : 0;
}

#ifndef _countof
template< typename T, size_t N >
char(*count_of_helper_(T(&)[N]))[N];
#define _countof( a ) (sizeof( *count_of_helper_( a ) ))
#endif

_START_ORPC_NAMESPACE_

void GetTimeSecond(int64_t* curTime);

void GetTimeMillSecond(int64_t* curTime);

std::string CurrentNamespaceName(const std::string& prettyFunction);

std::vector<std::string> tokenize_param_name(const char* str);

std::vector<std::string> tokenize_property_name(const char* str);

_END_ORPC_NAMESPACE_

#endif  //! #ifndef _BASE_UTIL_H_
