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

#ifndef ORPC_SERIALIZATION_H_
#define ORPC_SERIALIZATION_H_

#include "../build_config.h"
#include "stdint.h"
#include "../type_traits/is_container.h"
#include "../type_traits/function_traits.h"
#include "../property.h"
#include "../id_generator.h"
#include "../buff_stream.h"

_START_ORPC_NAMESPACE_

template<typename Func, std::size_t I, typename T>
typename std::enable_if <I == function_traits<Func>::nargs, bool>::type PackParam(msgpack::packer<BufferStream>& packer, const std::vector<std::string>& param_name, const T& args) { return true; }

template<typename Func, std::size_t I, typename T>
typename std::enable_if < I < function_traits<Func>::nargs, bool>::type PackParam(msgpack::packer<BufferStream>& packer, const std::vector<std::string>& param_name, const T& args);

template<typename Func, typename... Args>
uint64_t CheckAndSerialization(std::stringstream& s, const std::string& method_type, const std::string& method_name, const std::vector<std::string>& param_name, Args ...args);

template<typename Func>
uint64_t CheckAndSerialization(std::stringstream& s, const std::string& method_type, const std::string& method_name, const std::vector<std::string>& param_name);

template<typename T>
bool SerializationOther(msgpack::packer<BufferStream>& packer, const T& t);

//for enum
template<typename T>
typename std::enable_if<std::is_enum<T>::value, bool>::type SerializationEnum(msgpack::packer<BufferStream>& packer, const T& t);

template<typename T>
typename std::enable_if<!std::is_enum<T>::value, bool>::type SerializationEnum(msgpack::packer<BufferStream>& packer, const T& t);

//for cstring
template<typename T>
typename std::enable_if<std::is_same<T, const char*>::value, bool>::type SerializationCString(msgpack::packer<BufferStream>& packer, const T& t);

template<typename T>
typename std::enable_if<!std::is_same<T, const char*>::value, bool>::type SerializationCString(msgpack::packer<BufferStream>& packer, const T& t);

//for array
template<typename T>
typename std::enable_if<std::is_array<T>::value, bool>::type SerializationArray(msgpack::packer<BufferStream>& packer, const T& t);

template<typename T>
typename std::enable_if<!std::is_array<T>::value, bool>::type SerializationArray(msgpack::packer<BufferStream>& packer, const T& t);

//for BinaryString
template<typename T>
typename std::enable_if<std::is_same<T, BinaryString>::value, bool>::type SerializationBinaryString(msgpack::packer<BufferStream>& packer, const T& t);

template<typename T>
typename std::enable_if<!std::is_same<T, BinaryString>::value, bool>::type SerializationBinaryString(msgpack::packer<BufferStream>& packer, const T& t);

//for std::string
template<typename T>
typename std::enable_if<std::is_same<T, std::string>::value, bool>::type SerializationStdString(msgpack::packer<BufferStream>& packer, const T& t);

template<typename T>
typename std::enable_if<!std::is_same<T, std::string>::value, bool>::type SerializationStdString(msgpack::packer<BufferStream>& packer, const T& t);

//for model
template<typename T>
typename std::enable_if<is_model<T>::value, bool>::type SerializationModel(msgpack::packer<BufferStream>& packer, const T& t);

template<typename T>
typename std::enable_if<!is_model<T>::value, bool>::type SerializationModel(msgpack::packer<BufferStream>& packer, const T& t);

template<typename T>
typename std::enable_if<is_map_container<T>::value, bool>::type SerializationMap(msgpack::packer<BufferStream>& packer, const T& t);

template<typename T>
typename std::enable_if<!is_map_container<T>::value, bool>::type SerializationMap(msgpack::packer<BufferStream>& packer, const T& t);

template<typename T>
typename std::enable_if<is_seq_container<T>::value, bool>::type Serialization(msgpack::packer<BufferStream>& packer, const T& t);

template<typename T>
typename std::enable_if<!is_seq_container<T>::value, bool>::type Serialization(msgpack::packer<BufferStream>& packer, const T& t);

bool SerializationCallHeader(msgpack::packer<BufferStream>& packer, int32_t session_id, int32_t method_type, const std::string& method_name);

bool SerializationResultHeader(msgpack::packer<BufferStream>& packer, int32_t msg_type, int32_t session_id, int32_t error_id);

bool SerializationAuthHeader(msgpack::packer<BufferStream>& packer, int32_t app_id, int64_t session_yoken, int32_t privilege);

#include "serialization.inl"

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_SERIALIZATION_H_
