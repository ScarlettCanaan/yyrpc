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

#ifndef PARAM2MSGPACK_H_
#define PARAM2MSGPACK_H_

#include "../build_config.h"

#ifdef ORPC_SUPPROT_HTTP

#include <string>
#include "convert_def.h"
#include "../buff_stream.h"

_START_ORPC_NAMESPACE_

enum ParamTokenType
{
  ePTT_NIL,
  ePTT_TRUE,
  ePTT_FALSE,
  ePTT_NUMBER,
  ePTT_INT64,
  ePTT_STRING
};

struct ParamToken
{
  ParamToken(const std::string& key_, const std::string& val_);
  std::string key;
  std::string val;
  bool b;
  int64_t i;
  double d;
  ParamTokenType type;
};

struct Param2MsgPack
{
public:
  Param2MsgPack() {}
  bool Convert();
private:
  bool ConvertElement(ParamToken& value, msgpack::packer<BufferStream>& packer);
  bool WriteString(const char* data, size_t length, msgpack::packer<BufferStream>& packer);
public:
  std::string in_string;
  std::string out_string;
};

_END_ORPC_NAMESPACE_

#endif //! #ifdef ORPC_SUPPROT_HTTP

#endif  //! #ifndef PARAM2MSGPACK_H_
