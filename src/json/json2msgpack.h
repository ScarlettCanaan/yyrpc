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

#ifndef JSON2MSGPACK_H_
#define JSON2MSGPACK_H_

#include "../build_config.h"

#ifdef ORPC_SUPPROT_HTTP

#include <string>
#include <errno.h>
#include "convert_def.h"
#include "../buff_stream.h"

_START_ORPC_NAMESPACE_

struct Json2MsgPack
{
public:
  Json2MsgPack() { lax = false; use_float = false; }
  bool Convert();
private:
  bool WriteString(const char* data, size_t length, msgpack::packer<BufferStream>& packer);
  bool WriteExtString(const char* data, size_t length, msgpack::packer<BufferStream>& packer);
  bool WriteBinString(const char* data, size_t length, msgpack::packer<BufferStream>& packer);
  bool ConvertElement(Value& value, msgpack::packer<BufferStream>& packer);
public:
  std::string in_string;
  std::string out_string;
  bool lax;
  bool use_float;
};

_END_ORPC_NAMESPACE_

#endif //! #ifdef ORPC_SUPPROT_HTTP

#endif  //! #ifndef JSON2MSGPACK_H_
