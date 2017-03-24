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

#include "base_util.h"

_START_ORPC_NAMESPACE_

void GetTimeSecond(int64_t* curTime)
{
  if (!curTime)
    return;

  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> time_point;
  time_point = std::chrono::high_resolution_clock().now();
  int64_t t = time_point.time_since_epoch().count();
  t = t / 1000000000;
  *curTime = t;
}

void GetTimeMillSecond(int64_t* curTime)
{
  if (!curTime)
    return;

  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> time_point;
  time_point = std::chrono::high_resolution_clock().now();
  int64_t t = time_point.time_since_epoch().count();
  t = t / 1000000;
  *curTime = t;
}

std::string CurrentNamespaceName(const std::string& prettyFunction)
{
  size_t colonsA = prettyFunction.find("__namespace__");
  size_t colonsB = prettyFunction.rfind(" ", colonsA);
  if (colonsA <= colonsB)
    return "";

  ++colonsB;
  return prettyFunction.substr(colonsB, colonsA - colonsB);
}

std::string do_tokenize_param_name(const char* strPtr)
{
  while (*(strPtr - 1) == ' ')
    strPtr--;
  const char* nameEnd = strPtr;
  while (*strPtr != ' ' && *strPtr != '(')
    strPtr--;
  const char* nameFirst = strPtr + 1;
  return std::string(nameFirst, nameEnd - nameFirst);
}

std::vector<std::string> tokenize_param_name(const char* str)
{
  std::vector<std::string> tokens;
  const char* strPtr = str;

  bool haveValue = false;
  bool isInTemplate = false;
  bool haveLeftQ = false;
  while (*strPtr != 0)
  {
    if (*strPtr == '<')       isInTemplate = true;
    else if (*strPtr == '>')  isInTemplate = false;
    else if (*strPtr == '(')  haveLeftQ = true;

    haveValue |= (haveLeftQ && *strPtr != ' ' && *strPtr != ')' && *strPtr != '(');
    if ((*strPtr == ',' && !isInTemplate) || (haveValue && *strPtr == ')'))
      tokens.push_back(do_tokenize_param_name(strPtr));
    ++strPtr;
  }

  return tokens;
}

std::string do_tokenize_property_name(const char* firstPtr, const char* strPtr)
{
  while (*(strPtr - 1) == ' ')
    strPtr--;
  const char* nameEnd = strPtr;
  while (*strPtr != ' ' && strPtr != (firstPtr - 1))
    strPtr--;
  const char* nameFirst = strPtr + 1;
  return std::string(nameFirst, nameEnd - nameFirst);
}

std::vector<std::string> tokenize_property_name(const char* str)
{
  const char* strPtr = str;
  std::vector<std::string> tokens;
  while (*strPtr != 0)
  {
    if (*strPtr == ',')
      tokens.push_back(do_tokenize_property_name(str, strPtr));
    ++strPtr;
  }

  tokens.push_back(do_tokenize_property_name(str, strPtr));
  return tokens;
}

_END_ORPC_NAMESPACE_
