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

#include "unserialization.h"

_START_ORPC_NAMESPACE_

bool UnserializationCallHeader(const msgpack::object* obj, int32_t& session_id, int32_t& method_type, std::string& method_name)
{
  if (obj->type != msgpack::type::ARRAY)
    return false;

  msgpack::object_array v = obj->via.array;
  if (v.size != 3)
    return false;

  //method_type
  msgpack::object* o = v.ptr;
  if (o->type != msgpack::type::NEGATIVE_INTEGER && o->type != msgpack::type::POSITIVE_INTEGER)
    return false;
  method_type = o->as<int32_t>();

  //session_id
  o = v.ptr + 1;
  if (o->type != msgpack::type::NEGATIVE_INTEGER && o->type != msgpack::type::POSITIVE_INTEGER)
    return false;
  session_id = o->as<int32_t>();

  //method_name
  o = v.ptr + 2;
  if (o->type != msgpack::type::STR)
    return false;
  method_name = o->as<std::string>();
  return true;
}

bool UnserializationResultHeader(const msgpack::object* obj, int32_t& method_type, int32_t& session_id, int32_t& error_id, std::string& method_name)
{
  if (obj->type != msgpack::type::ARRAY)
    return false;

  msgpack::object_array v = obj->via.array;
  if (v.size != 3)
    return false;

  //msg_type
  msgpack::object* o = v.ptr;
  if (o->type != msgpack::type::NEGATIVE_INTEGER && o->type != msgpack::type::POSITIVE_INTEGER)
    return false;
  method_type = o->as<int32_t>();

  //session_id
  o = v.ptr + 1;
  if (o->type != msgpack::type::NEGATIVE_INTEGER && o->type != msgpack::type::POSITIVE_INTEGER)
    return false;
  session_id = o->as<int32_t>();

  o = v.ptr + 2;
  if (o->type == msgpack::type::NEGATIVE_INTEGER || o->type == msgpack::type::POSITIVE_INTEGER)
  {
    error_id = o->as<int32_t>();
  }
  else if (o->type == msgpack::type::STR)
  {
    method_name = o->as<std::string>();
  }
  else
  {
    ORPC_LOG(ERROR) << "only support 'result', 'fire', method_type: " << method_type;
    return false;
  }
  return true;
}

bool UnserializationAuthHeader(const msgpack::object* obj, int32_t& app_id, int64_t& session_yoken, int32_t& privilege)
{
  if (obj->type != msgpack::type::ARRAY)
    return false;

  msgpack::object_array v = obj->via.array;
  if (v.size != 3)
    return false;

  //app_id
  msgpack::object* o = v.ptr;
  if (o->type != msgpack::type::NEGATIVE_INTEGER && o->type != msgpack::type::POSITIVE_INTEGER)
    return false;
  app_id = o->as<int32_t>();

  //session_yoken
  o = v.ptr + 1;
  if (o->type != msgpack::type::NEGATIVE_INTEGER && o->type != msgpack::type::POSITIVE_INTEGER)
    return false;
  session_yoken = o->as<int64_t>();

  //privilege
  o = v.ptr + 2;
  if (o->type != msgpack::type::NEGATIVE_INTEGER && o->type != msgpack::type::POSITIVE_INTEGER)
    return false;
  privilege = o->as<int32_t>();
  return true;
}

_END_ORPC_NAMESPACE_
