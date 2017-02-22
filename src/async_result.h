#ifndef YY_ASYNC_RESULT_H_
#define YY_ASYNC_RESULT_H_

#include <unordered_map>
#include <future>
#include <exception>

class IAsyncResult
{
public:
  virtual ~IAsyncResult() {}
  virtual void set_result(void* r) = 0;
  virtual void set_exception(const std::string& str) = 0;
  virtual void call_on_result() = 0;
};

template<typename T>
class AsyncResult : public IAsyncResult
{
public:
  AsyncResult(uint32_t session_id) : m_sessionId(session_id), m_resultOk(false) {}
public:
  virtual void set_result(void* r) override;
  virtual void set_exception(const std::string& str) override;
  virtual void call_on_result() override;

  T wait() { return m_promise.get_future().get(); }
  
  void on_result(const std::function<void(const T&)>& callback) const;
private:
  uint32_t m_sessionId;
  std::promise<T> m_promise;
  bool m_resultOk;
  mutable std::function<void(const T&)> m_callback;
};

template<typename T>
void AsyncResult<T>::on_result(const std::function<void(const T&)>& callback) const
{
  m_callback = callback;
}

template<typename T>
void AsyncResult<T>::set_result(void* r)
{
  T* t = (T*)r;
  m_promise.set_value(*t);
  m_resultOk = true;
}

template<typename T>
void AsyncResult<T>::set_exception(const std::string& str)
{
  m_promise.set_exception(std::make_exception_ptr(std::logic_error(str)));
}

template<typename T>
void AsyncResult<T>::call_on_result()
{
  if (m_resultOk && m_callback)
    m_callback(m_promise.get_future().get());
}

template<>
class AsyncResult<void> : public IAsyncResult
{
public:
  AsyncResult(uint32_t session_id) : m_sessionId(session_id) {}
public:
  virtual void set_result(void* r) override {}
  virtual void set_exception(const std::string& str) override {}
  virtual void call_on_result() override {}
private:
  uint32_t m_sessionId;
};

#endif  //! #ifndef YY_ASYNC_RESULT_H_
