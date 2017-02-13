#ifndef YYRPC_PROPERTY_H_
#define YYRPC_PROPERTY_H_

#include <iostream>
#include <list>
#include <string>
#include <assert.h>
#include <type_traits>
#include <sstream>
#include "msgpack.hpp"

template<typename T>
struct is_string : public std::integral_constant <bool, std::is_same<char*, typename std::decay<T>::type>::value || std::is_same<const char*, typename std::decay<T>::type>::value > {};

template<>
struct is_string<std::string> : std::true_type{};

struct TRpcPropertyBase;

struct TRpcPropertyBase
{
public:
  TRpcPropertyBase(const std::string& n);

  const std::string& GetKey() const { return name; }

  virtual ~TRpcPropertyBase() = default;
  virtual bool Accept(void* obj, const msgpack::object& s) const { return false; };
  virtual std::string GetValue(void* obj) const { return ""; }
private:
  std::string name;
};

inline TRpcPropertyBase::TRpcPropertyBase(const std::string& n)
  : name(n)
{

}

template<typename M, typename T>
struct TRpcProperty : public TRpcPropertyBase
{
public:
  using PropertyType = T;

  TRpcProperty(const std::string& n, T M::* p);
public:
  virtual bool Accept(void* obj, const msgpack::object& s) const override;
  virtual std::string GetValue(void* obj) const override;
private:
  T M::* data;
};

template<typename M, typename T>
TRpcProperty<M, T>::TRpcProperty(const std::string& n, T M::* p)
  : TRpcPropertyBase(n), data(p)
{

}

template<typename M, typename T>
bool TRpcProperty<M, T>::Accept(void* obj, const msgpack::object& s) const
{
  M* m = (M*)obj;
  if (std::is_integral<T>() && (s.type != msgpack::type::POSITIVE_INTEGER && s.type != msgpack::type::NEGATIVE_INTEGER))
    return false;
  else if (std::is_floating_point<T>() && (s.type != msgpack::type::FLOAT32 && s.type != msgpack::type::FLOAT64))
    return false;
  else if (is_string<T>() && (s.type != msgpack::type::STR))
    return false;
  m->*data = s.as<typename std::decay<T>::type>();
  return true;
}

template<typename M, typename T>
std::string TRpcProperty<M, T>::GetValue(void* obj) const
{
  M* m = (M*)obj;
  std::ostringstream ss;
  if (is_string<T>())
    ss << "'" << m->*data << "'";
  else
    ss << m->*data;
  return std::move(ss.str());
}

template <typename M>
class TRpcModelBase
{
public:
  using ModelClass = M;
  static const char* ModelName;
  static const TRpcPropertyBase* PropertyList[];
  static const size_t PropertyListCount;
};

// Mark T::ModelName as a model
template<typename T> struct is_model : std::integral_constant<bool, std::is_base_of <TRpcModelBase<T>, T>::value> {};

//0
#define CHOOSE_HELPER1(model) 

//1
#define CHOOSE_HELPER2(model, member) \
  new TRpcProperty<model, decltype(model::member)>{#member, &model::member},

//2
#define CHOOSE_HELPER3(model, member, member2) \
  new TRpcProperty<model, decltype(model::member)>{#member, &model::member}, \
  new TRpcProperty<model, decltype(model::member2)>{#member2, &model::member2},

//3
#define CHOOSE_HELPER4(model, member, member2, member3) \
  new TRpcProperty<model, decltype(model::member)>{#member, &model::member},\
  new TRpcProperty<model, decltype(model::member2)>{#member2, &model::member2},\
  new TRpcProperty<model, decltype(model::member3)>{#member3, &model::member3},

//4
#define CHOOSE_HELPER5(model, member, member2, member3, member4) \
  new TRpcProperty<model, decltype(model::member)>{#member, &model::member},\
  new TRpcProperty<model, decltype(model::member2)>{#member2, &model::member2},\
  new TRpcProperty<model, decltype(model::member3)>{#member3, &model::member3},\
  new TRpcProperty<model, decltype(model::member4)>{#member4, &model::member4},

//5
#define CHOOSE_HELPER6(model, member, member2, member3, member4, member5) \
  new TRpcProperty<model, decltype(model::member)>{#member, &model::member},\
  new TRpcProperty<model, decltype(model::member2)>{#member2, &model::member2},\
  new TRpcProperty<model, decltype(model::member3)>{#member3, &model::member3},\
  new TRpcProperty<model, decltype(model::member4)>{#member4, &model::member4},\
  new TRpcProperty<model, decltype(model::member5)>{#member5, &model::member5},

//6
#define CHOOSE_HELPER7(model, member, member2, member3, member4, member5, member6) \
  new TRpcProperty<model, decltype(model::member)>{#member, &model::member},\
  new TRpcProperty<model, decltype(model::member2)>{#member2, &model::member2},\
  new TRpcProperty<model, decltype(model::member3)>{#member3, &model::member3},\
  new TRpcProperty<model, decltype(model::member4)>{#member4, &model::member4},\
  new TRpcProperty<model, decltype(model::member5)>{#member5, &model::member5},\
  new TRpcProperty<model, decltype(model::member6)>{#member6, &model::member6},

//7
#define CHOOSE_HELPER8(model, member, member2, member3, member4, member5, member6, member7) \
  new TRpcProperty<model, decltype(model::member)>{#member, &model::member},\
  new TRpcProperty<model, decltype(model::member2)>{#member2, &model::member2},\
  new TRpcProperty<model, decltype(model::member3)>{#member3, &model::member3},\
  new TRpcProperty<model, decltype(model::member4)>{#member4, &model::member4},\
  new TRpcProperty<model, decltype(model::member5)>{#member5, &model::member5},\
  new TRpcProperty<model, decltype(model::member6)>{#member6, &model::member6},\
  new TRpcProperty<model, decltype(model::member7)>{#member7, &model::member7},

/*
* Count the number of arguments passed to ASSERT, very carefully
* tiptoeing around an MSVC bug where it improperly expands __VA_ARGS__ as a
* single token in argument lists.  See these URLs for details:
*
*   http://connect.microsoft.com/VisualStudio/feedback/details/380090/variadic-macro-replacement
*/
#define COUNT_CHOOSE_ARGS_IMPL8(_1, _2, _3, _4, _5, _6, _7, _8, count, ...) \
   count
#define COUNT_CHOOSE_ARGS_IMPL7(args) \
   COUNT_CHOOSE_ARGS_IMPL8 args
#define COUNT_CHOOSE_ARGS_IMPL6(args) \
   COUNT_CHOOSE_ARGS_IMPL7 (args)
#define COUNT_CHOOSE_ARGS_IMPL5(args) \
   COUNT_CHOOSE_ARGS_IMPL6 (args)
#define COUNT_CHOOSE_ARGS_IMPL4(args) \
   COUNT_CHOOSE_ARGS_IMPL5 (args)
#define COUNT_CHOOSE_ARGS_IMPL3(args) \
   COUNT_CHOOSE_ARGS_IMPL4 (args)
#define COUNT_CHOOSE_ARGS_IMPL2(args) \
   COUNT_CHOOSE_ARGS_IMPL3 (args)
#define COUNT_CHOOSE_ARGS_IMPL(args) \
   COUNT_CHOOSE_ARGS_IMPL2 (args)

#define COUNT_CHOOSE_ARGS(...) \
   COUNT_CHOOSE_ARGS_IMPL((__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0))

/* Pick the right helper macro to invoke. */
#define CHOOSE_CHOOSE_HELPER8(count) CHOOSE_HELPER##count
#define CHOOSE_CHOOSE_HELPER7(count) CHOOSE_CHOOSE_HELPER8(count)
#define CHOOSE_CHOOSE_HELPER6(count) CHOOSE_CHOOSE_HELPER7(count)
#define CHOOSE_CHOOSE_HELPER5(count) CHOOSE_CHOOSE_HELPER6(count)
#define CHOOSE_CHOOSE_HELPER4(count) CHOOSE_CHOOSE_HELPER5(count)
#define CHOOSE_CHOOSE_HELPER3(count) CHOOSE_CHOOSE_HELPER4(count)
#define CHOOSE_CHOOSE_HELPER2(count) CHOOSE_CHOOSE_HELPER3(count)
#define CHOOSE_CHOOSE_HELPER1(count) CHOOSE_CHOOSE_HELPER2(count)
#define CHOOSE_CHOOSE_HELPER(count)  CHOOSE_CHOOSE_HELPER1(count)

/* The actual macro. */
#define CHOOSE_GLUE(x, y) x y

#define YYRPC_MODEL_REGISTER(model, ...) static_assert(std::is_base_of<TRpcModelBase<model>, model>::value, "RPC_CALL_ERROR: model must inherit from TRpcModelBase<model>!"); template<> const char* TRpcModelBase<model>::ModelName = #model; \
                                     template<> const TRpcPropertyBase* TRpcModelBase<model>::PropertyList[] = {CHOOSE_GLUE(CHOOSE_CHOOSE_HELPER(COUNT_CHOOSE_ARGS(model, __VA_ARGS__)), \
               (model, __VA_ARGS__)) new TRpcPropertyBase{""} }; template<> const size_t TRpcModelBase<model>::PropertyListCount = std::extent<decltype(TRpcModelBase<model>::PropertyList) >::value - 1;;

#endif  //! #ifndef YYRPC_PROPERTY_H_
