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

template<typename Func, std::size_t I, typename T>
typename std::enable_if<I < function_traits<Func>::nargs, bool>::type PackParam(msgpack::packer<BufferStream>& packer, const std::vector<std::string>& param_name, const T& args)
{
  using define_type = typename function_traits<Func>::template argument<I>::type;
  using raw_type = typename std::remove_cv<typename std::remove_reference<define_type>::type>::type;

  using param_type = typename std::tuple_element<I, T>::type;
  static_assert(std::is_convertible<param_type, define_type>::value, "RPC_CALL_ERROR: argument type mismatch.");
  if (param_name.size() > I)
    packer.pack(param_name[I]);
  else
    packer.pack("unnamed");
  bool r = Serialization<raw_type>(packer, std::get<I>(args));
  if (!r)
    return false;
  return PackParam<Func, I + 1, T>(packer, param_name, args);
}

template<typename Func, typename... Args>
uint64_t CheckAndSerialization(BufferStream& s, int32_t method_type, const std::string& method_name, const std::vector<std::string>& param_name, Args ...args)
{
  uint64_t session_id = GenNextSessionId();
  msgpack::packer<BufferStream> packer(s);
  packer.pack_map(2);
  packer.pack(0);
  if (!SerializationCallHeader(packer, session_id, method_type, method_name))
    return -1;
  packer.pack(1);
  using traits = function_traits < Func > ;
  static_assert(sizeof...(args) == traits::nargs, "RPC_CALL_ERROR: argument count mismatch.");
  packer.pack_map(traits::nargs);
  if (!PackParam<Func, 0>(packer, param_name, std::make_tuple(args...)))
    return -1;

  return session_id;
}

template<typename Func>
uint64_t CheckAndSerialization(BufferStream& s, int32_t method_type, const std::string& method_name, const std::vector<std::string>& param_name)
{
  uint64_t session_id = GenNextSessionId();
  msgpack::packer<BufferStream> packer(s);
  packer.pack_map(2);
  packer.pack(0);
  if (!SerializationCallHeader(packer, session_id, method_type, method_name))
    return -1;
  packer.pack(1);
  packer.pack_nil();
  using traits = function_traits < Func > ;
  static_assert(traits::nargs == 0, "RPC_CALL_ERROR: argument count mismatch.");
  return session_id;
}

template<typename T>
bool SerializationOther(msgpack::packer<BufferStream>& packer, const T& t)
{
  static_assert(!std::is_class<T>::value, "RPC_CALL_ERROR: struct not register!");
  static_assert(!std::is_function<T>::value, "RPC_CALL_ERROR: Serialization don't support function!");
  static_assert(!std::is_union<T>::value, "RPC_CALL_ERROR: Serialization don't support union!");
  static_assert(!std::is_pointer<T>::value, "RPC_CALL_ERROR: Serialization don't support pointer!");

  packer.pack(t);
  return true;
}

//for enum
template<typename T>
typename std::enable_if<std::is_enum<T>::value, bool>::type SerializationEnum(msgpack::packer<BufferStream>& packer, const T& t)
{
  packer.pack((int32_t)t);
  return true;
}

template<typename T>
typename std::enable_if<!std::is_enum<T>::value, bool>::type SerializationEnum(msgpack::packer<BufferStream>& packer, const T& t)
{
  return SerializationOther(packer, t);
}

//for cstring
template<typename T>
typename std::enable_if<std::is_same<T, const char*>::value, bool>::type SerializationCString(msgpack::packer<BufferStream>& packer, const T& t)
{
  packer.pack(t);
  return true;
}

template<typename T>
typename std::enable_if<!std::is_same<T, const char*>::value, bool>::type SerializationCString(msgpack::packer<BufferStream>& packer, const T& t)
{
  return SerializationEnum(packer, t);
}

//for array
template<typename T>
typename std::enable_if<std::is_array<T>::value, bool>::type SerializationArray(msgpack::packer<BufferStream>& packer, const T& t)
{
  using first_dimension = typename std::remove_extent<T>::type;
  using deepest_dimension = typename std::remove_all_extents<T>::type;
  static_assert(std::is_same<first_dimension, deepest_dimension>::value, "RPC_CALL_ERROR: array only support one-dimensional!");
  packer.pack_array(std::extent<T>::value);
  for (int i = 0; i < std::extent<T>::value; ++i)
  {
    using raw_type = typename std::remove_cv<typename std::remove_reference<first_dimension>::type>::type;
    Serialization<raw_type>(packer, t[i]);
  }
  return true;
}

template<typename T>
typename std::enable_if<!std::is_array<T>::value, bool>::type SerializationArray(msgpack::packer<BufferStream>& packer, const T& t)
{
  return SerializationCString(packer, t);
}

//for BinaryString
template<typename T>
typename std::enable_if<std::is_same<T, BinaryString>::value, bool>::type SerializationBinaryString(msgpack::packer<BufferStream>& packer, const T& t)
{
  packer.pack_bin(t.data.length());
  packer.pack_bin_body(t.data.c_str(), t.data.length());
  return true;
}

template<typename T>
typename std::enable_if<!std::is_same<T, BinaryString>::value, bool>::type SerializationBinaryString(msgpack::packer<BufferStream>& packer, const T& t)
{
  return SerializationArray(packer, t);
}

//for std::string
template<typename T>
typename std::enable_if<std::is_same<T, std::string>::value, bool>::type SerializationStdString(msgpack::packer<BufferStream>& packer, const T& t)
{
  packer.pack(t);
  return true;
}

template<typename T>
typename std::enable_if<!std::is_same<T, std::string>::value, bool>::type SerializationStdString(msgpack::packer<BufferStream>& packer, const T& t)
{
  return SerializationBinaryString(packer, t);
}

template<std::size_t I = 0, typename Tuple>
typename std::enable_if<I == std::tuple_size<Tuple>::value, void>::type serialization_tuple(msgpack::packer<BufferStream>& packer, const std::vector<std::string>& meta_name, const Tuple& t)
{
}

template<std::size_t I = 0, typename Tuple>
typename std::enable_if <I < std::tuple_size<Tuple>::value, void>::type serialization_tuple(msgpack::packer<BufferStream>& packer, const std::vector<std::string>& meta_name, const Tuple& t)
{
  if (meta_name.size() > I)
    packer.pack(meta_name[I]);
  else
    packer.pack("unnamed");
  Serialization(packer, std::get<I>(t));
  serialization_tuple<I + 1>(packer, meta_name, t);
}

//for model
template<typename T>
typename std::enable_if<is_model<T>::value, bool>::type SerializationModel(msgpack::packer<BufferStream>& packer, const T& t)
{
  auto tuple = const_cast<T&>(t).PropertyMeta();
  const std::vector<std::string>& metaNames = const_cast<T&>(t).PropertyMetaVName();
  packer.pack_map(std::tuple_size<decltype(tuple)>::value);
  serialization_tuple(packer, metaNames, tuple);
  return true;
}

template<typename T>
typename std::enable_if<!is_model<T>::value, bool>::type SerializationModel(msgpack::packer<BufferStream>& packer, const T& t)
{
  return SerializationStdString(packer, t);
}

template<typename T>
typename std::enable_if<is_map_container<T>::value, bool>::type SerializationMap(msgpack::packer<BufferStream>& packer, const T& t)
{
  packer.pack_map(t.size());
  for (auto it : t)
  {
    using raw_key_type = typename std::remove_cv<typename std::remove_reference<typename T::key_type>::type>::type;
    Serialization<raw_key_type>(packer, it.first);
    using raw_value_type = typename std::remove_cv<typename std::remove_reference<typename T::mapped_type>::type>::type;
    Serialization<raw_value_type>(packer, it.second);
  }
  return true;
}

template<typename T>
typename std::enable_if<!is_map_container<T>::value, bool>::type SerializationMap(msgpack::packer<BufferStream>& packer, const T& t)
{
  return SerializationModel(packer, t);
}

template<typename T>
typename std::enable_if<is_seq_container<T>::value, bool>::type Serialization(msgpack::packer<BufferStream>& packer, const T& t)
{
  packer.pack_array(t.size());
  for (auto it : t)
  {
    using raw_type = typename std::remove_cv<typename std::remove_reference<typename T::value_type>::type>::type;
    Serialization<raw_type>(packer, it);
  }
  return true;
}

template<typename T>
typename std::enable_if<!is_seq_container<T>::value, bool>::type Serialization(msgpack::packer<BufferStream>& packer, const T& t)
{
  return SerializationMap(packer, t);
}
