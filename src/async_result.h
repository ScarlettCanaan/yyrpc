#ifndef YY_ASYNC_RESULT_H_
#define YY_ASYNC_RESULT_H_

#include <unordered_map>
#include <future>
#include <exception>
#include "util/util.h"
#include "msgpack.hpp"
#include "error_def.h"
#include "st.h"
#include "build_config.h"

struct this_thread_t
{};

struct any_thread_t
{};

class IAsyncResult
{
public:
  virtual ~IAsyncResult() {}
public:
  virtual bool set_result(msgpack::unpacker& unp) = 0;
  virtual bool set_exception(int32_t error) = 0;
  virtual void run_callback() const = 0;
  virtual bool is_timeouted(int64_t cur_time) = 0;
  virtual std::thread::id get_run_thread_id() const = 0;
};

template<typename T>
class AsyncResult : public IAsyncResult
{
public:
  AsyncResult();
  ~AsyncResult();

public:
  virtual bool set_result(msgpack::unpacker& unp) override;
  virtual bool set_exception(int32_t error) override;
  virtual void run_callback() const override;
  virtual bool is_timeouted(int64_t cur_time) override;
  virtual std::thread::id get_run_thread_id() const override { return m_callThreadId; };

public: // only for AsyncResultWrapper
  T NOTHROW wait();
  st_cond_t NOTHROW get_fiber_cond();
  
  void set_call_thread_id(std::thread::id id) { m_callThreadId = id; }
  void set_result_callback(const std::function<void(const T&)>& callback);
  void set_exception_callback(const std::function<void(int32_t)>& callback);
  void set_fiber_callback(const std::function<void(void)>& callback);

  void set_timeout_value(int64_t value) { m_timeoutValue = value; }

  int32_t get_error() const { return m_errorNo; }
private:
  int32_t m_errorNo;
  int64_t m_calltimeValue;
  int64_t m_timeoutValue;
  mutable std::promise<T> m_promise;
  T  value;
  std::function<void(const T&)> m_resultCallback;
  std::function<void(int32_t)> m_exceptionCallback;
  std::thread::id m_callThreadId;
  st_cond_t m_fibeCond;
  std::function<void(void)> m_fiberCallback;
};

template<typename T>
AsyncResult<T>::AsyncResult() :
m_timeoutValue(3000),
m_errorNo(YYRPC_ERROR_UNKNOWN),
m_fibeCond(0)
{
  GetTimeMillSecond(&m_calltimeValue);
}

template<typename T>
AsyncResult<T>::~AsyncResult()
{
  if (m_fibeCond)
    st_cond_destroy(m_fibeCond);
}

template<typename T>
T AsyncResult<T>::wait()
{
  if (m_fibeCond)
  {
    if (st_cond_timedwait(m_fibeCond, m_timeoutValue * 1000) < 0)
      m_errorNo = YYRPC_ERROR_CALL_TIMEOUT;
  }

  if (m_errorNo == YYRPC_ERROR_SUCESS)
  {
    return value;
  }
  else if (m_errorNo > 0)
  {
#ifdef WIN32
    return T();
#else
    std::runtime_error(error_id_to_string(m_errorNo));
#endif // WIN32
  }
  else
  {
    return m_promise.get_future().get();
  }

  //not reached
  return T();
}

template<typename T>
st_cond_t AsyncResult<T>::get_fiber_cond()
{
  if (!m_fibeCond)
    m_fibeCond = st_cond_new();
  return m_fibeCond;
}

template<typename T>
void AsyncResult<T>::set_result_callback(const std::function<void(const T&)>& callback)
{
  m_resultCallback = callback;
}

template<typename T>
void AsyncResult<T>::set_exception_callback(const std::function<void(int32_t)>& callback)
{
  m_exceptionCallback = callback;
}

template<typename T>
void AsyncResult<T>::set_fiber_callback(const std::function<void(void)>& callback)
{
  m_fiberCallback = callback;
}

template<typename T>
bool AsyncResult<T>::set_result(msgpack::unpacker& unp)
{
  if (m_errorNo != YYRPC_ERROR_UNKNOWN)
    return false;

  unserialization(unp, value);
  m_errorNo = YYRPC_ERROR_SUCESS;
  m_promise.set_value(value);
  return true;
}

template<typename T>
bool AsyncResult<T>::set_exception(int32_t error)
{
  if (m_errorNo != YYRPC_ERROR_UNKNOWN)
    return false;

  m_errorNo = error;
  m_promise.set_exception(std::make_exception_ptr(std::runtime_error(error_id_to_string(error))));
  return true;
}

template<typename T>
void AsyncResult<T>::run_callback() const
{
  if (m_errorNo == YYRPC_ERROR_SUCESS && m_fiberCallback) //fiber must call first
    m_fiberCallback();

  if (m_errorNo > 0 && m_exceptionCallback)
    m_exceptionCallback(m_errorNo);

  if (m_errorNo == YYRPC_ERROR_SUCESS && m_resultCallback)
    m_resultCallback(value);
}

template<typename T>
bool AsyncResult<T>::is_timeouted(int64_t cur_time)
{
  if (m_errorNo != YYRPC_ERROR_UNKNOWN)
    return false;

  if (cur_time < m_calltimeValue ||
    cur_time - m_calltimeValue < m_timeoutValue)
    return false;

  set_exception(YYRPC_ERROR_CALL_TIMEOUT);
  return true;
}

template<>
class AsyncResult<void> : public IAsyncResult
{
public:
  virtual bool set_result(msgpack::unpacker& unp) override { return true; };
  virtual bool set_exception(int32_t error) override { return true; }
  virtual void run_callback() const override {}
  virtual bool is_timeouted(int64_t cur_time) override { return true; };
  virtual std::thread::id get_run_thread_id() const override { return std::thread::id(); };
};

template<typename R>
struct AsyncResultWrapper
{
public:
  AsyncResultWrapper(int errorNo) : m_errorNo(errorNo) {}
  AsyncResultWrapper(const std::shared_ptr<AsyncResult<R>>& r) : result(r), m_errorNo(YYRPC_ERROR_UNKNOWN) {}
public:
  template <typename T = this_thread_t>
  AsyncResultWrapper<R> then(const std::function<void(const R&)>& callback);

  template <typename T = this_thread_t>
  AsyncResultWrapper<R> then(const std::function<void(const R&)>& callback, const std::function<void(int32_t)>& timeout);

  R wait();

  operator bool() const { return result.get() != 0; }
  operator R () { return wait(); }

  AsyncResultWrapper<R> set_timeout_value(int64_t value);

  int32_t get_error() const;
private:
  AsyncResultWrapper<R> set_fiber_callback(const std::function<void(void)>& callback);
  st_cond_t NOTHROW get_fiber_cond();
  friend class CallerManager;
private:
  std::shared_ptr<AsyncResult<R>> result;
  int32_t m_errorNo;
};

template<typename R>
template <typename T>
AsyncResultWrapper<R> AsyncResultWrapper<R>::then(const std::function<void(const R&)>& callback)
{
  if (!result)
    return *this;

  if (std::is_same<T, this_thread_t>::value)
    result->set_call_thread_id(std::this_thread::get_id());
  result->set_result_callback(callback);
  return *this;
}

template<typename R>
template <typename T>
AsyncResultWrapper<R> AsyncResultWrapper<R>::then(const std::function<void(const R&)>& callback, const std::function<void(int32_t)>& exception)
{
  if (!result)
    return *this;

  if (std::is_same<T, this_thread_t>::value)
    result->set_call_thread_id(std::this_thread::get_id());
  result->set_result_callback(callback);
  result->set_exception_callback(exception);
  return *this;
}

template<typename R>
AsyncResultWrapper<R> AsyncResultWrapper<R>::set_fiber_callback(const std::function<void(void)>& callback)
{
  if (!result)
    return *this;

  result->set_fiber_callback(callback);
  return *this;
}

template<typename R>
R AsyncResultWrapper<R>::wait()
{
  if (!result)
  {
#ifdef WIN32
    return R();
#else
    throw std::runtime_error("null obj.");
#endif  //! #ifdef WIN32
  }
    
  return result->wait();
}

template<typename R>
st_cond_t AsyncResultWrapper<R>::get_fiber_cond()
{
  return result->get_fiber_cond();
}

template<typename R>
AsyncResultWrapper<R> AsyncResultWrapper<R>::set_timeout_value(int64_t value)
{
  if (!result)
    return *this;

  result->set_timeout_value(value);
  return *this;
}

template<typename R>
int32_t AsyncResultWrapper<R>::get_error() const
{
  if (!result)
    return m_errorNo;

  return result->get_error();
}

template<>
struct AsyncResultWrapper<void>
{
public:
  AsyncResultWrapper(int errorNo) : m_errorNo(errorNo) {}
  AsyncResultWrapper(const std::shared_ptr<AsyncResult<void>>& r) : m_errorNo(YYRPC_ERROR_SUCESS) {}
public:
  int32_t get_error() const { return m_errorNo; }
private:
  int32_t m_errorNo;
};

#endif  //! #ifndef YY_ASYNC_RESULT_H_
