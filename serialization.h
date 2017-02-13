#ifndef YYRPC_SERIALIZATION_H_
#define YYRPC_SERIALIZATION_H_

#include "type_traits/is_container.h"

template<typename T>
bool serialization_other(std::stringstream& s, const T& t)
{
  static_assert(!std::is_class<T>::value, "RPC_CALL_ERROR: struct not register!");
  static_assert(!std::is_function<T>::value, "RPC_CALL_ERROR: serialization don't support function!");
  static_assert(!std::is_union<T>::value, "RPC_CALL_ERROR: serialization don't support union!");
  static_assert(!std::is_pointer<T>::value, "RPC_CALL_ERROR: serialization don't support pointer!");
  return true;
}

//for array
template<typename T>
typename std::enable_if<std::is_array<T>::value, bool>::type serialization_array(std::stringstream& s, const T& t)
{
  static_assert(std::is_same<std::remove_extent<T>::type, std::remove_all_extents<T>::type>::value, "RPC_CALL_ERROR: array only support one-dimensional!");
  int array_size = std::extent<T>::value;
  for (int i = 0; i < array_size; ++i)
  {
    auto v = t[i];
    using raw_type = typename std::remove_cv<typename std::remove_reference<std::remove_extent<T>::type>::type>::type;
    serialization<raw_type>(s, v);
  }
  return true;
}

template<typename T>
typename std::enable_if<!std::is_array<T>::value, bool>::type serialization_array(std::stringstream& s, const T& t)
{
  return serialization_other(s, t);
}

//for std::string
template<typename T>
typename std::enable_if<std::is_same<T, std::string>::value, bool>::type serialization_string(std::stringstream& s, const T& t)
{
  return true;
}

template<typename T>
typename std::enable_if<!std::is_same<T, std::string>::value, bool>::type serialization_string(std::stringstream& s, const T& t)
{
  return serialization_array(s, t);
}

//for model
template<typename T>
typename std::enable_if<is_model<T>::value, bool>::type serialization_model(std::stringstream& s, const T& t)
{
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
  //msgpack::pack(s, args.get<I>());
  return true;
}

template<typename T>
typename std::enable_if<!is_container<T>::value, bool>::type serialization(std::stringstream& s, const T& t)
{
  return serialization_model(s, t);
}

#endif  //! #ifndef YYRPC_SERIALIZATION_H_
