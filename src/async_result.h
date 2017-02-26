#ifndef YY_ASYNC_RESULT_H_
#define YY_ASYNC_RESULT_H_

#include <unordered_map>
#include <future>
#include <exception>
#include "util/util.h"
#include "msgpack.hpp"
#include "error_def.h"
#include "caller_manager.h"

struct this_thread
{};

struct any_thread
{};


class IAsyncResult
{
public:
  virtual ~IAsyncResult() {}
  virtual bool set_result(msgpack::unpacker& unp) = 0;
  virtual void set_exception(int32_t error) = 0;
  virtual void run_callback() const = 0;
  virtual bool check_timeout(int64_t cur_time) = 0;
};

template<typename T>
class AsyncResult : public IAsyncResult, public std::enable_shared_from_this<AsyncResult<T>>
{
public:
  AsyncResult(uint64_t session_id) : 
    m_sessionId(session_id), 
    m_bResultOk(false),
    m_bException(false),
    m_timeout(3000),
    m_errorId(0)
  { 
    GetTimeMillSecond(&m_calltime);
  }
public:
  virtual bool set_result(msgpack::unpacker& unp) override;
  virtual void set_exception(int32_t error) override;
  virtual void run_callback() const override;
  virtual bool check_timeout(int64_t cur_time) override;

  T wait() { return m_promise.get_future().get(); }
  
  void set_call_thread_id(std::thread::id id) { m_callThreadId = id; }
  void set_on_result(const std::function<void(const T&)>& callback);
  void set_on_exception(const std::function<void(int32_t)>& callback);
private:
  int64_t m_calltime;
  int64_t m_timeout;
  int32_t m_errorId;
  uint64_t m_sessionId;
  mutable std::promise<T> m_promise;
  bool m_bResultOk;
  bool m_bException;
  std::function<void(const T&)> m_resultCallback;
  std::function<void(int32_t)> m_exceptionCallback;
  std::thread::id m_callThreadId;
};

template<typename T>
void AsyncResult<T>::set_on_result(const std::function<void(const T&)>& callback)
{
  m_resultCallback = callback;
}

template<typename T>
void AsyncResult<T>::set_on_exception(const std::function<void(int32_t)>& callback)
{
  m_exceptionCallback = callback;
}

template<typename T>
bool AsyncResult<T>::set_result(msgpack::unpacker& unp)
{
  T t;
  unserialization(unp, t);
  m_promise.set_value(t);
  m_bResultOk = true;
  if (m_resultCallback && !m_bException)
    CallerManager::GetInstance().OnResult(m_callThreadId, AsyncResult<T>::shared_from_this());
  return true;
}

template<typename T>
void AsyncResult<T>::set_exception(int32_t error)
{
  m_bException = true;
  m_errorId = error;
  m_promise.set_exception(std::make_exception_ptr(std::runtime_error(error_id_to_string(error))));
  if (m_exceptionCallback && !m_bResultOk)
    CallerManager::GetInstance().OnResult(m_callThreadId, AsyncResult<T>::shared_from_this());
}

template<typename T>
void AsyncResult<T>::run_callback() const
{
  if (m_bException && m_exceptionCallback)
    m_exceptionCallback(m_errorId);

  if (m_bResultOk && m_resultCallback)
    m_resultCallback(m_promise.get_future().get());
}

template<typename T>
bool AsyncResult<T>::check_timeout(int64_t cur_time)
{
  if (cur_time < m_calltime)
    return false;
  if (cur_time - m_calltime < m_timeout)
    return false;
  set_exception(YYRPC_ERROR_CALL_TIMEOUT);
  return true;
}

template<>
class AsyncResult<void> : public IAsyncResult
{
public:
  AsyncResult(uint64_t session_id) {}
  virtual bool set_result(msgpack::unpacker& unp) override { return true; };
  virtual void set_exception(int32_t error) override {}
  virtual void run_callback() const override {}
  virtual bool check_timeout(int64_t cur_time) override { return true; };
};

template<typename R>
struct AsyncResultWrapper
{
public:
  AsyncResultWrapper() {}
  AsyncResultWrapper(const std::shared_ptr<AsyncResult<R>>& r) : result(r) {}
public:
  template <typename T = this_thread>
  AsyncResultWrapper<R> then(const std::function<void(const R&)>& callback);

  template <typename T = this_thread>
  AsyncResultWrapper<R> then(const std::function<void(const R&)>& callback, const std::function<void(int32_t)>& timeout);

  R wait();

  operator bool() const { return result.get() != 0; }
private:
  std::shared_ptr<AsyncResult<R>> result;
};

template<typename R>
template <typename T>
AsyncResultWrapper<R> AsyncResultWrapper<R>::then(const std::function<void(const R&)>& callback)
{
  if (!result)
    return *this;
  if (std::is_same<T, this_thread>::value)
    result->set_call_thread_id(std::this_thread::get_id());
  result->set_on_result(callback);
  return *this;
}

template<typename R>
template <typename T>
AsyncResultWrapper<R> AsyncResultWrapper<R>::then(const std::function<void(const R&)>& callback, const std::function<void(int32_t)>& exception)
{
  if (!result)
    return *this;
  if (std::is_same<T, this_thread>::value)
    result->set_call_thread_id(std::this_thread::get_id());
  result->set_on_result(callback);
  result->set_on_exception(exception);
  return *this;
}

template<typename R>
R AsyncResultWrapper<R>::wait()
{
  if (result)
    return result->wait();
  throw std::runtime_error("wait on null obj.");
}

template<>
struct AsyncResultWrapper<void>
{
  AsyncResultWrapper() {}
  AsyncResultWrapper(const std::shared_ptr<AsyncResult<void>>& r) {}
};

#endif  //! #ifndef YY_ASYNC_RESULT_H_
