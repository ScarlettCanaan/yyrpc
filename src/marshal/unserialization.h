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

#ifndef ORPC_UNSERIALIZATION_H_
#define ORPC_UNSERIALIZATION_H_

#include "../build_config.h"
#include "stdint.h"
#include "../type_traits/is_container.h"
#include "../type_traits/function_traits.h"
#include "../property.h"

_START_ORPC_NAMESPACE_

template<typename Func, std::size_t I, typename T>
typename std::enable_if <I == function_traits<Func>::nargs, bool>::type UnpackParam(const msgpack::object_kv* kv, T& args) { return true; }

template<typename Func, std::size_t I, typename T>
typename std::enable_if < I < function_traits<Func>::nargs, bool>::type UnpackParam(const msgpack::object_kv* kv, T& args);

template<typename... Args>
bool CheckAndUnSerialization(const msgpack::object* obj, std::tuple<Args ...>& args);

template<typename T>
bool UnserializationOther(const msgpack::object* obj, T& t);

//for enum
template<typename T>
typename std::enable_if<std::is_enum<T>::value, bool>::type UnserializationEnum(const msgpack::object* obj, T& t);

template<typename T>
typename std::enable_if<!std::is_enum<T>::value, bool>::type UnserializationEnum(const msgpack::object* obj, T& t);

//for array
template<typename T>
typename std::enable_if<std::is_array<T>::value, bool>::type UnserializationArray(const msgpack::object* obj, T& t);

template<typename T>
typename std::enable_if<!std::is_array<T>::value, bool>::type UnserializationArray(const msgpack::object* obj, T& t);

//for BinaryString
template<typename T>
typename std::enable_if<std::is_same<T, BinaryString>::value, bool>::type UnserializationBinaryString(const msgpack::object* obj, T& t);

template<typename T>
typename std::enable_if<!std::is_same<T, BinaryString>::value, bool>::type UnserializationBinaryString(const msgpack::object* obj, T& t);

//for std::string
template<typename T>
typename std::enable_if<std::is_same<T, std::string>::value, bool>::type UnserializationStdString(const msgpack::object* obj, T& t);

template<typename T>
typename std::enable_if<!std::is_same<T, std::string>::value, bool>::type UnserializationStdString(const msgpack::object* obj, T& t);

//for model
template<typename T>
typename std::enable_if<is_model<T>::value, bool>::type UnserializationModel(const msgpack::object* obj, T& t);

template<typename T>
typename std::enable_if<!is_model<T>::value, bool>::type UnserializationModel(const msgpack::object* obj, T& t);

//map
template<typename T>
typename std::enable_if<is_map_container<T>::value, bool>::type UnserializationMap(const msgpack::object* obj, T& t);

template<typename T>
typename std::enable_if<!is_map_container<T>::value, bool>::type UnserializationMap(const msgpack::object* obj, T& t);

template<typename T>
typename std::enable_if<is_seq_container<T>::value, bool>::type Unserialization(const msgpack::object* obj, T& t);

template<typename T>
typename std::enable_if<!is_seq_container<T>::value, bool>::type Unserialization(const msgpack::object* obj, T& t);

bool UnserializationCallHeader(const msgpack::object* obj, int32_t& session_id, int32_t& method_type, std::string& method_name);

bool UnserializationResultHeader(const msgpack::object* obj, int32_t& method_type, int32_t& session_id, int32_t& error_id, std::string& method_name);

bool UnserializationAuthHeader(const msgpack::object* obj, int32_t& app_id, int64_t& session_yoken, int32_t& privilege);

#include "unserialization.inl"

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_UNSERIALIZATION_H_
