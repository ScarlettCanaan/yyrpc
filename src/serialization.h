#ifndef YYRPC_SERIALIZATION_H_
#define YYRPC_SERIALIZATION_H_

#include "stdint.h"
#include "type_traits/is_container.h"
#include "property.h"
#include "msgpack.hpp"

template<typename T>
typename std::enable_if<is_container<T>::value, bool>::type serialization(std::stringstream& s, const T& t);

template<typename T>
typename std::enable_if<!is_container<T>::value, bool>::type serialization(std::stringstream& s, const T& t);


template<typename T>
bool serialization_other(std::stringstream& s, const T& t)
{
  static_assert(!std::is_class<T>::value, "RPC_CALL_ERROR: struct not register!");
  static_assert(!std::is_function<T>::value, "RPC_CALL_ERROR: serialization don't support function!");
  static_assert(!std::is_union<T>::value, "RPC_CALL_ERROR: serialization don't support union!");
  static_assert(!std::is_pointer<T>::value, "RPC_CALL_ERROR: serialization don't support pointer!");
  msgpack::pack(s, t);
  return true;
}

//for cstring
template<typename T>
typename std::enable_if<std::is_same<T, const char*>::value, bool>::type serialization_cstring(std::stringstream& s, const T& t)
{
  msgpack::pack(s, t);
  return true;
}

template<typename T>
typename std::enable_if<!std::is_same<T, const char*>::value, bool>::type serialization_cstring(std::stringstream& s, const T& t)
{
  return serialization_other(s, t);
}

//for array
template<typename T>
typename std::enable_if<std::is_array<T>::value, bool>::type serialization_array(std::stringstream& s, const T& t)
{
  using first_dimension = typename std::remove_extent<T>::type;
  using deepest_dimension = typename std::remove_all_extents<T>::type;
  static_assert(std::is_same<first_dimension, deepest_dimension>::value, "RPC_CALL_ERROR: array only support one-dimensional!");
  int array_size = std::extent<T>::value;
  for (int i = 0; i < array_size; ++i)
  {
    using raw_type = typename std::remove_cv<typename std::remove_reference<first_dimension>::type>::type;
    serialization<raw_type>(s, t[i]);
  }
  return true;
}

template<typename T>
typename std::enable_if<!std::is_array<T>::value, bool>::type serialization_array(std::stringstream& s, const T& t)
{
  return serialization_cstring(s, t);
}

//for std::string
template<typename T>
typename std::enable_if<std::is_same<T, std::string>::value, bool>::type serialization_string(std::stringstream& s, const T& t)
{
  msgpack::pack(s, t);
  return true;
}

template<typename T>
typename std::enable_if<!std::is_same<T, std::string>::value, bool>::type serialization_string(std::stringstream& s, const T& t)
{
  return serialization_array(s, t);
}

template<std::size_t I = 0, typename Tuple>
typename std::enable_if<I == std::tuple_size<Tuple>::value>::type serialization_tuple(std::stringstream& s, const Tuple& t)
{
}

template<std::size_t I = 0, typename Tuple>
typename std::enable_if < I < std::tuple_size<Tuple>::value>::type serialization_tuple(std::stringstream& s, const Tuple& t)
{
  serialization(s, std::get<I>(t));
  serialization_tuple<I + 1>(s, t);
}

//for model
template<typename T>
typename std::enable_if<is_model<T>::value, bool>::type serialization_model(std::stringstream& s, const T& t)
{
  auto tuple = ((T&)t).meta();
  serialization_tuple(s, tuple);
  return true;
}

template<typename T>
typename std::enable_if<!is_model<T>::value, bool>::type serialization_model(std::stringstream& s, const T& t)
{
  return serialization_string(s, t);
}

template<typename T>
typename std::enable_if<is_container<T>::value, bool>::type serialization(std::stringstream& s, const T& t)
{
  msgpack::pack(s, t);
  return true;
}

template<typename T>
typename std::enable_if<!is_container<T>::value, bool>::type serialization(std::stringstream& s, const T& t)
{
  return serialization_model(s, t);
}

inline bool serialization_header(std::stringstream& s, int32_t session_id, std::string& method_name)
{
  msgpack::pack(s, session_id);
  msgpack::pack(s, method_name);
  return true;
}

#endif  //! #ifndef YYRPC_SERIALIZATION_H_
