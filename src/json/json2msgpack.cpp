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

#include "json2msgpack.h"

#ifdef ORPC_SUPPROT_HTTP

_START_ORPC_NAMESPACE_

static char* DecoderBase64(const char* p, size_t len, size_t* out_bytes) 
{
  size_t bytes = len * 3 / 4 + 4;
  char* data = (char*)malloc(bytes);
  if (!data)
    return NULL;
  base64_decodestate state;
  base64_init_decodestate(&state);
  bytes = base64_decode_block(p, len, data, &state);
  *out_bytes = bytes;
  return data;
}

bool Json2MsgPack::WriteExtString(const char* data, size_t length, msgpack::packer<BufferStream>& packer)
{
  const char* exttype_str = data + prefix_ext_length;
  char* remainder;
  int64_t exttype = strtol(exttype_str, &remainder, 10);
  if (exttype < INT8_MIN || exttype > INT8_MAX) {
    ORPC_LOG(ERROR) << "string prefixed with \"ext:\" has out-of-bounds ext type: " << exttype;
    return false;
  }

  const char* base64_data = remainder;
  size_t base64_len = length - (base64_data - data);
  size_t count;
  char* bytes = DecoderBase64(base64_data, base64_len, &count);
  if (!bytes)
    return false;
  packer.pack_ext(count, (int8_t)exttype);
  packer.pack_ext_body(bytes, count);
  free(bytes);
  return true;
}

bool Json2MsgPack::WriteBinString(const char* data, size_t length, msgpack::packer<BufferStream>& packer)
{
  size_t count;
  const char* bin_str = data + prefix_base64_length;
  char* bytes = DecoderBase64(bin_str, length - prefix_base64_length, &count);
  if (!bytes)
    return false;
  packer.pack_bin(count);
  packer.pack_bin_body(bytes, count);
  free(bytes);
  return true;
}

bool Json2MsgPack::WriteString(const char* data, size_t length, msgpack::packer<BufferStream>& packer)
{
  if (length >= prefix_ext_length && memcmp(data, prefix_ext, prefix_ext_length) == 0)
  {
    return WriteExtString(data, length, packer);
  }
  else if (length >= prefix_base64_length && memcmp(data, prefix_base64, prefix_base64_length) == 0)
  {
    return WriteBinString(data, length, packer);
  }

  packer.pack_str(length);
  packer.pack_str_body(data, length);
  return true;
}

bool Json2MsgPack::ConvertElement(Value& value, msgpack::packer<BufferStream>& packer)
{
  switch (value.GetType())
  {
  case kNullType:   packer.pack_nil();    break;
  case kTrueType:   packer.pack_true();   break;
  case kFalseType:  packer.pack_false();  break;

  case kNumberType:
    if (value.IsDouble())
    {
      if (use_float)
        packer.pack_float((float)value.GetDouble());
      else
        packer.pack_double(value.GetDouble());
    }
    else if (value.IsUint64())
    {
      packer.pack_uint64(value.GetUint64());
    }
    else
    {
      packer.pack_int64(value.GetInt64());
    }
    break;

  case kStringType:
    if (!WriteString(value.GetString(), value.GetStringLength(), packer))
      return false;
    break;

  case kArrayType: {
    packer.pack_array(value.Size());
    for (Value::ValueIterator it = value.Begin(); it != value.End(); ++it)
    {
      if (!ConvertElement(*it, packer))
        return false;
    }
    break;
  }

  case kObjectType: {
    packer.pack_map(value.MemberCount());
    for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it)
    {
      if (!WriteString(it->name.GetString(), it->name.GetStringLength(), packer))
        return false;
      if (!ConvertElement(it->value, packer))
        return false;
    }
    break;
  }

  default:
    return false;
  }

  return true;
}

bool Json2MsgPack::Convert()
{
  BufferStream s;
  msgpack::packer<BufferStream> packer(s);

  char* in_data = (char*)in_string.c_str();
  Document document;
  if (lax)
    document.ParseInsitu<kParseFullPrecisionFlag | kParseCommentsFlag | kParseTrailingCommasFlag>(in_data);
  else
    document.ParseInsitu<kParseFullPrecisionFlag>(in_data);

  if (document.HasParseError())
  {
    ORPC_LOG(ERROR) << "error parsing JSON at offset:" << (int)document.GetErrorOffset() << " error:" << GetParseError_En(document.GetParseError());
    return false;
  }

  if (!ConvertElement(document, packer))
    return false;

  out_string = std::string(s.data(), s.size());
  return true;
}

_END_ORPC_NAMESPACE_

#endif  //! #ifdef ORPC_SUPPROT_HTTP
