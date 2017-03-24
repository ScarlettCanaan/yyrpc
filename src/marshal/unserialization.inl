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
typename std::enable_if < I < function_traits<Func>::nargs, bool>::type UnpackParam(const msgpack::object_kv* kv, T& args)
{
  using define_type = typename function_traits<Func>::template argument<I>::type;
  using raw_type = typename std::remove_cv<typename std::remove_reference<define_type>::type>::type;

  using param_type = typename std::tuple_element<I, T>::type;
  static_assert(std::is_convertible<param_type, define_type>::value, "RPC_CALL_ERROR: argument type mismatch.");
  const msgpack::object_kv* this_kv = kv + I;
  bool r = Unserialization<raw_type>(&this_kv->val, std::get<I>(args));
  if (!r)
    return false;
  return UnpackParam<Func, I + 1, T>(kv, args);
}

template<typename... Args>
bool CheckAndUnSerialization(const msgpack::object* obj, std::tuple<Args ...>& args)
{
  if (obj->type != msgpack::type::MAP)
    return false;

  msgpack::object_map v = obj->via.map;
  if (v.size != sizeof...(Args))
    return false;

  bool ret = false;
  try
  {
    ret = UnpackParam<void(Args...), 0>(v.ptr, args);
  }
  catch (msgpack::type_error&)
  {
    ORPC_LOG(ERROR) << "CheckAndUnSerialization failed with msgpack::type_error.";
  }
  catch (...)
  {
    ORPC_LOG(ERROR) << "CheckAndUnSerialization failed with unknown error.";
  }

  return ret;
}

template<typename T>
bool UnserializationOther(const msgpack::object* obj, T& t)
{
  static_assert(!std::is_class<T>::value, "RPC_CALL_ERROR: struct not register!");
  static_assert(!std::is_function<T>::value, "RPC_CALL_ERROR: Unserialization don't support function!");
  static_assert(!std::is_union<T>::value, "RPC_CALL_ERROR: Unserialization don't support union!");
  static_assert(!std::is_pointer<T>::value, "RPC_CALL_ERROR: Unserialization don't support pointer!");
  t = obj->as<typename std::decay<T>::type>();
  return true;
}

//for enum
template<typename T>
typename std::enable_if<std::is_enum<T>::value, bool>::type UnserializationEnum(const msgpack::object* obj, T& t)
{
  int32_t et = obj->as<int32_t>();
  t = (T)et;
  return true;
}

template<typename T>
typename std::enable_if<!std::is_enum<T>::value, bool>::type UnserializationEnum(const msgpack::object* obj, T& t)
{
  return UnserializationOther(obj, t);
}

//for array
template<typename T>
typename std::enable_if<std::is_array<T>::value, bool>::type UnserializationArray(const msgpack::object* obj, T& t)
{
  if (obj->type != msgpack::type::ARRAY)
    return false;

  msgpack::object_array v = obj->via.array;
  if (v.size != std::extent<T>::value)
    return false;

  using first_dimension = typename std::remove_extent<T>::type;
  using deepest_dimension = typename std::remove_all_extents<T>::type;
  static_assert(std::is_same<first_dimension, deepest_dimension>::value, "RPC_CALL_ERROR: array only support one-dimensional!");
  for (size_t i = 0; i < v.size; ++i)
  {
    using raw_type = typename std::remove_cv<typename std::remove_reference<first_dimension>::type>::type;
    Unserialization<raw_type>(obj, t[i]);
  }
  return true;
}

template<typename T>
typename std::enable_if<!std::is_array<T>::value, bool>::type UnserializationArray(const msgpack::object* obj, T& t)
{
  return UnserializationEnum(obj, t);
}

//for BinaryString
template<typename T>
typename std::enable_if<std::is_same<T, BinaryString>::value, bool>::type UnserializationBinaryString(const msgpack::object* obj, T& t)
{
  if (obj->type == msgpack::type::BIN && obj->type != msgpack::type::STR)
    return false;

  if (obj->type == msgpack::type::BIN)
  {
    msgpack::object_bin bin = obj->via.bin;
    t.data = std::string(bin.ptr, bin.size);
  }
  else if (obj->type == msgpack::type::STR)
  {
    msgpack::object_str str = obj->via.str;
    t.data = std::string(str.ptr, str.size);
  }

  return true;
}

template<typename T>
typename std::enable_if<!std::is_same<T, BinaryString>::value, bool>::type UnserializationBinaryString(const msgpack::object* obj, T& t)
{
  return UnserializationArray(obj, t);
}

//for std::string
template<typename T>
typename std::enable_if<std::is_same<T, std::string>::value, bool>::type UnserializationStdString(const msgpack::object* obj, T& t)
{
  t = obj->as<typename std::decay<T>::type>();
  return true;
}

template<typename T>
typename std::enable_if<!std::is_same<T, std::string>::value, bool>::type UnserializationStdString(const msgpack::object* obj, T& t)
{
  return UnserializationBinaryString(obj, t);
}

template<std::size_t I = 0, typename Tuple>
typename std::enable_if<I == std::tuple_size<Tuple>::value>::type unserialization_tuple(const msgpack::object_kv* kv, Tuple& t)
{
}

template<std::size_t I = 0, typename Tuple>
typename std::enable_if < I < std::tuple_size<Tuple>::value>::type unserialization_tuple(const msgpack::object_kv* kv, Tuple& t)
{
  const msgpack::object_kv* this_kv = kv + I;
  Unserialization(&this_kv->val, std::get<I>(t));
  unserialization_tuple<I + 1>(kv, t);
}

//for model
template<typename T>
typename std::enable_if<is_model<T>::value, bool>::type UnserializationModel(const msgpack::object* obj, T& t)
{
  auto tuple = t.PropertyMeta();
  if (obj->type != msgpack::type::MAP)
    return false;

  msgpack::object_map v = obj->via.map;
  if (v.size != std::tuple_size<decltype(t.PropertyMeta())>::value)
    return false;

  unserialization_tuple(v.ptr, tuple);

  return true;
}

template<typename T>
typename std::enable_if<!is_model<T>::value, bool>::type UnserializationModel(const msgpack::object* obj, T& t)
{
  return UnserializationStdString(obj, t);
}

//map
template<typename T>
typename std::enable_if<is_map_container<T>::value, bool>::type UnserializationMap(const msgpack::object* obj, T& t)
{
  if (obj->type != msgpack::type::MAP)
    return false;

  msgpack::object_map v = obj->via.map;
  for (size_t i = 0; i < v.size; ++i)
  {
    msgpack::object_kv* kv = v.ptr + i;
    using raw_key_type = typename std::remove_cv<typename std::remove_reference<typename T::key_type>::type>::type;
    raw_key_type raw_key;
    Unserialization<raw_key_type>(&kv->key, raw_key);
    using raw_value_type = typename std::remove_cv<typename std::remove_reference<typename T::mapped_type>::type>::type;
    raw_key_type raw_value;
    Unserialization<raw_key_type>(&kv->val, raw_value);
    t[raw_key] = raw_value;
  }
  return true;
}

template<typename T>
typename std::enable_if<!is_map_container<T>::value, bool>::type UnserializationMap(const msgpack::object* obj, T& t)
{
  return UnserializationModel(obj, t);
}

template<typename T>
typename std::enable_if<is_seq_container<T>::value, bool>::type Unserialization(const msgpack::object* obj, T& t)
{
  if (obj->type != msgpack::type::ARRAY)
    return false;

  msgpack::object_array v = obj->via.array;
  for (size_t i = 0; i < v.size; ++i)
  {
    using raw_type = typename std::remove_cv<typename std::remove_reference<typename T::value_type>::type>::type;
    raw_type raw;
    Unserialization<raw_type>(v.ptr + i, raw);
    t.push_back(raw);
  }
  return true;
}

template<typename T>
typename std::enable_if<!is_seq_container<T>::value, bool>::type Unserialization(const msgpack::object* obj, T& t)
{
  return UnserializationMap(obj, t);
}

