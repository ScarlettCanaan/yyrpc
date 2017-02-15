#ifndef YY_ASYNC_RESULT_H_
#define YY_ASYNC_RESULT_H_

#include <hash_map>
#include <future>
#include <exception>

class IAsyncResult
{
public:
  virtual ~IAsyncResult() {}
  virtual void SetResult(void* r) = 0;
  virtual void SetException(const std::string& str) = 0;
};

template<typename T>
class AsyncResult : public IAsyncResult
{
public:
  AsyncResult(uint32_t session_id) : m_sessionId(session_id) {}
public:
  virtual void SetResult(void* r) override
  {
    T* t = (T*)r;
    m_promise.set_value(*t);
  }
  virtual void SetException(const std::string& str) override
  {
    m_promise.set_exception(std::make_exception_ptr(std::logic_error(str)));
  }
  T Wait() { return m_promise.get_future().get(); }
private:
  uint32_t m_sessionId;
  std::promise<T> m_promise;
};

#endif  //! #ifndef YY_ASYNC_RESULT_H_
