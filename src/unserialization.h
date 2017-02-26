#ifndef YYRPC_UNSERIALIZATION_H_
#define YYRPC_UNSERIALIZATION_H_

#include "stdint.h"
#include "type_traits/is_container.h"
#include "property.h"
#include "msgpack.hpp"

template<typename T>
typename std::enable_if<is_container<T>::value, bool>::type unserialization(msgpack::unpacker& unp, T& t);

template<typename T>
typename std::enable_if<!is_container<T>::value, bool>::type unserialization(msgpack::unpacker& unp, T& t);

template<typename T>
bool unserialization_other(msgpack::unpacker& unp, T& t)
{
  static_assert(!std::is_class<T>::value, "RPC_CALL_ERROR: struct not register!");
  static_assert(!std::is_function<T>::value, "RPC_CALL_ERROR: unserialization don't support function!");
  static_assert(!std::is_union<T>::value, "RPC_CALL_ERROR: unserialization don't support union!");
  static_assert(!std::is_pointer<T>::value, "RPC_CALL_ERROR: unserialization don't support pointer!");
  msgpack::object_handle o;
  if (!unp.next(o))
    return false;
  t = o.get().as<typename std::decay<T>::type>();
  return true;
}

//for array
template<typename T>
typename std::enable_if<std::is_array<T>::value, bool>::type unserialization_array(msgpack::unpacker& unp, T& t)
{
  using first_dimension = typename std::remove_extent<T>::type;
  using deepest_dimension = typename std::remove_all_extents<T>::type;
  static_assert(std::is_same<first_dimension, deepest_dimension>::value, "RPC_CALL_ERROR: array only support one-dimensional!");
  for (int i = 0; i < std::extent<T>::value; ++i)
  {
    using raw_type = typename std::remove_cv<typename std::remove_reference<first_dimension>::type>::type;
    unserialization<raw_type>(unp, t[i]);
  }
  return true;
}

template<typename T>
typename std::enable_if<!std::is_array<T>::value, bool>::type unserialization_array(msgpack::unpacker& unp, T& t)
{
  return unserialization_other(unp, t);
}

//for std::string
template<typename T>
typename std::enable_if<std::is_same<T, std::string>::value, bool>::type unserialization_string(msgpack::unpacker& unp, T& t)
{
  msgpack::object_handle o;
  if (!unp.next(o))
    return false;

  t = o.get().as<typename std::decay<T>::type>();
  return true;
}

template<typename T>
typename std::enable_if<!std::is_same<T, std::string>::value, bool>::type unserialization_string(msgpack::unpacker& unp, T& t)
{
  return unserialization_array(unp, t);
}

template<std::size_t I = 0, typename Tuple>
typename std::enable_if<I == std::tuple_size<Tuple>::value>::type unserialization_tuple(msgpack::unpacker& unp, Tuple& t)
{
}

template<std::size_t I = 0, typename Tuple>
typename std::enable_if < I < std::tuple_size<Tuple>::value>::type unserialization_tuple(msgpack::unpacker& unp, Tuple& t)
{
  unserialization(unp, std::get<I>(t));
  unserialization_tuple<I + 1>(unp, t);
}

//for model
template<typename T>
typename std::enable_if<is_model<T>::value, bool>::type unserialization_model(msgpack::unpacker& unp, T& t)
{
  auto tuple = t.meta();
  unserialization_tuple(unp, tuple);

  return true;
}

template<typename T>
typename std::enable_if<!is_model<T>::value, bool>::type unserialization_model(msgpack::unpacker& unp, T& t)
{
  return unserialization_string(unp, t);
}

template<typename T>
typename std::enable_if<is_container<T>::value, bool>::type unserialization(msgpack::unpacker& unp, T& t)
{
  msgpack::object_handle o;
  if (!unp.next(o))
    return false;
  t = o.get().as<typename std::decay<T>::type>();
  return true;
}

template<typename T>
typename std::enable_if<!is_container<T>::value, bool>::type unserialization(msgpack::unpacker& unp, T& t)
{
  return unserialization_model(unp, t);
}

inline bool unserialization_header(msgpack::unpacker& unp, int32_t& session_id, std::string& method_name)
{
  msgpack::object_handle o;
  if (!unp.next(o))
    return false;
  session_id = o.get().as<int32_t>();

  if (!unp.next(o))
    return false;
  method_name = o.get().as<std::string>();
  return true;
}

#endif  //! #ifndef YYRPC_UNSERIALIZATION_H_
