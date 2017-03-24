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

// The code that checks that object keys are strings just uses
// RAPIDJSON_ASSERT(). We define it to nothing to disable this
// so we can write non-string keys in debug mode. (We actually
// never call Key(), and always write whatever type we want.)
#define RAPIDJSON_ASSERT(x) ((void)(x))

#include "msgpack2json.h"

#ifdef ORPC_SUPPROT_HTTP

#include <sstream>

_START_ORPC_NAMESPACE_

template <class WriterType>
bool WriteString(const msgpack::object* obj, WriterType& writer)
{
  assert(obj->type == msgpack::type::STR);
  std::string s = obj->as<std::string>();
  return writer.String(s.c_str(), s.length());
}

static const char* EncodeBase64(const std::string& str, const std::string& prefix, size_t* out_bytes)
{
  uint32_t new_len = prefix.length() + ((str.length() + 3) * 4) / 3;
  char* output = (char*)malloc(new_len);
  if (!output)
    return NULL;

  char* p = output;
  memcpy(p, prefix.c_str(), prefix.length());
  p += prefix.length();
  
  base64_encodestate state;
  base64_init_encodestate(&state);
  p += base64_encode_block(str.c_str(), (int)str.length(), p, &state);
  p += base64_encode_blockend(p, &state);

  *out_bytes = p - output;
  return output;
}

template <class WriterType>
bool WriteBase64Bin(const msgpack::object* obj, WriterType& writer)
{
  std::string s = obj->as<std::string>();
  size_t count = 0;
  const char* bytes = EncodeBase64(s, prefix_base64, &count);
  if (!bytes)
    return false;
  writer.String(bytes, count);
  free((void*)bytes);
  return true;
}

template <class WriterType>
bool WriteBase64Ext(const msgpack::object* obj, WriterType& writer)
{
  msgpack::object_ext v = obj->via.ext;
  char buffer[64] = { 0 };
  sprintf(buffer, "%s%i", prefix_ext, v.type());
  std::string s = obj->as<std::string>();
  size_t count = 0;
  const char* bytes = EncodeBase64(s, buffer, &count);
  if (!bytes)
    return false;
  writer.String(bytes, count);
  free((void*)bytes);
  return true;
}

template <class WriterType>
bool ConvertElement(const msgpack::object* obj, WriterType& writer, MsgPack2Json& converter)
{
  switch (obj->type)
  {
  case msgpack::type::BOOLEAN:   return writer.Bool(obj->as<bool>());
  case msgpack::type::NIL:    return writer.Null();
  case msgpack::type::POSITIVE_INTEGER:    return writer.Int64(obj->as<int64_t>());
  case msgpack::type::NEGATIVE_INTEGER:   return writer.Uint64(obj->as<uint64_t>());
  case msgpack::type::FLOAT32:  return writer.Double(obj->as<float>());
  case msgpack::type::FLOAT64: return writer.Double(obj->as<double>());

  case msgpack::type::STR:
    return WriteString(obj, writer);

  case msgpack::type::BIN:
    if (converter.debug)
    {
      std::string s = obj->as<std::string>();
      char buf[64];
      snprintf(buf, sizeof(buf), "<bin of size %u>", s.length());
      return writer.RawValue(buf, strlen(buf), kStringType);
    }
    return WriteBase64Bin(obj, writer);

  case msgpack::type::EXT:
    if (converter.debug)
    {
      msgpack::object_ext v = obj->via.ext;
      char buf[64];
      snprintf(buf, sizeof(buf), "<ext of type %i size %u>", v.type(), v.size);
      return writer.RawValue(buf, strlen(buf), kStringType);
    }
    return WriteBase64Ext(obj, writer);

  case msgpack::type::ARRAY:
    if (!writer.StartArray())
      return false;
    for (size_t i = 0; i < obj->via.array.size; ++i)
    {
      msgpack::object* o = obj->via.array.ptr + i;
      ConvertElement(o, writer, converter);
    }
    return writer.EndArray();

  case msgpack::type::MAP:
    if (!writer.StartObject())
      return false;
    for (size_t i = 0; i < obj->via.map.size; ++i)
    {
      msgpack::object_kv* o = obj->via.map.ptr + i;
      ConvertElement(&o->key, writer, converter);
      ConvertElement(&o->val, writer, converter);
    }
    return writer.EndObject();
  }

  return true;
}

template <class WriterType>
bool ConvertFirstElements(msgpack::unpacker& unp, WriterType& writer, StringBuffer& stream, MsgPack2Json& converter)
{
  msgpack::object_handle result;
  bool firstElement = true;
  if (!unp.next(result))
    return false;

  if (!ConvertElement(&result.get(), writer, converter))
    return false;

  return true;
}

bool MsgPack2Json::Convert()
{
  msgpack::unpacker unp;
  unp.reserve_buffer(in_string.size());
  std::istringstream input(in_string);
  std::size_t actual_read_size = input.readsome(unp.buffer(), in_string.size());
  if (actual_read_size != in_string.size())
    return false;

  // tell msgpack::unpacker actual consumed size.
  unp.buffer_consumed(actual_read_size);

  bool ret;
  StringBuffer stream;
  if (pretty)
  {
    PrettyWriter<StringBuffer> writer(stream);
    ret = ConvertFirstElements(unp, writer, stream, *this);
    stream.Put('\n');
    stream.Flush();
  }
  else
  {
    Writer<StringBuffer> writer(stream);
    ret = ConvertFirstElements(unp, writer, stream, *this);
  }

  if (!ret)
    ORPC_LOG(ERROR) << "msgpack2json error.";
  else
    out_string = std::string(stream.GetString(), stream.GetSize());
  
  return ret;
}

_END_ORPC_NAMESPACE_

#endif  //! #ifdef ORPC_SUPPROT_HTTP
