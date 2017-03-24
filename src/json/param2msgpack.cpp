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

#include "param2msgpack.h"

#ifdef ORPC_SUPPROT_HTTP

#include <errno.h>
#include "../util/base_util.h"

_START_ORPC_NAMESPACE_

ParamToken::ParamToken(const std::string& key_, const std::string& val_)
  : key(key_), val(val_)
{
  if (val.empty())
  {
    type = ePTT_NIL;
    return;
  }
  else if (stricmp(val.c_str(), "true") == 0)
  {
    b = true;
    type = ePTT_TRUE;
    return;
  }
  else if (stricmp(val.c_str(), "false") == 0)
  {
    b = false;
    type = ePTT_FALSE;
    return;
  }
  for (auto ch : val)
  {
    if (!isdigit(ch) && ch != '.')
    {
      type = ePTT_STRING;
      return;
    }
    else if (ch == '.')
    {
      type = ePTT_NUMBER;
      d = StringToNumber<double>(val);
      return;
    }
  }

  type = ePTT_INT64;
  i = StringToNumber<int64_t>(val);
}

bool Param2MsgPack::WriteString(const char* data, size_t length, msgpack::packer<BufferStream>& packer)
{
  packer.pack_str(length);
  packer.pack_str_body(data, length);
  return true;
}

bool Param2MsgPack::ConvertElement(ParamToken& value, msgpack::packer<BufferStream>& packer)
{
  packer.pack_str(value.key.length());
  packer.pack_str_body(value.key.c_str(), value.key.length());
  switch (value.type)
  {
  case ePTT_NIL:   packer.pack_nil();    break;
  case ePTT_TRUE:   packer.pack_true();   break;
  case ePTT_FALSE:  packer.pack_false();  break;

  case ePTT_NUMBER:
    packer.pack_double(value.d); break;
  case ePTT_INT64:
    packer.pack_int64(value.i); break;

  case ePTT_STRING:
    if (!WriteString(value.val.c_str(), value.val.length(), packer))
      return false;
    break;
  default:
    return false;
  }

  return true;
}

static bool do_parser_http_token(std::vector<ParamToken>& v, const char* data, size_t& begin, size_t& equal, size_t& end)
{
  if (equal < begin || equal == 0)
    return false;

  if (end - equal - 1 > 0)
  {
    ParamToken token(std::string(data + begin, equal - begin), std::string(data + equal + 1, end - equal - 1));
    v.push_back(token);
  }
  else
  {
    ParamToken token(std::string(data + begin, equal - begin), "");
    v.push_back(token);
  }
  equal = 0;
  begin = end + 1;
  return true;
}

static std::vector<ParamToken> parser_http_token(const char* data, size_t len)
{
  std::vector<ParamToken> v;

  size_t pos = 0;
  size_t begin = 0;
  size_t equal = 0;
  for (; pos != len; ++pos)
  {
    if (*(data + pos) == '=')
      equal = pos;
    else if (*(data + pos) == '&' && !do_parser_http_token(v, data, begin, equal, pos))
      return v;
  }

  do_parser_http_token(v, data, begin, equal, pos);
  return v;
}

bool Param2MsgPack::Convert()
{
  BufferStream s;
  msgpack::packer<BufferStream> packer(s);

  std::vector<ParamToken> tokens = parser_http_token(in_string.c_str(), in_string.length());
  packer.pack_map(tokens.size());
  for (auto it : tokens)
  {
    if (!ConvertElement(it, packer))
      return false;
  }

  out_string = std::string(s.data(), s.size());
  return true;
}

_END_ORPC_NAMESPACE_

#endif //! #ifdef ORPC_SUPPROT_HTTP
