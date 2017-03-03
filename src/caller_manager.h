#ifndef YY_CALL_MANAGER_H_
#define YY_CALL_MANAGER_H_

#include <map>
#include <memory>
#include <list>
#include <thread>
#include <mutex>
#include <functional>
#include <unordered_map>
#include "st.h"
#include "build_config.h"
#include "async_result.h"

struct FunctionWrapper
{
  std::function<int(void)> func;
};

class IAsyncResult;

class CallerManager
{
public:
  static CallerManager& GetInstance();
public:
  int Init(bool useFiber, size_t maxFiberNum);
  int UnInit();
private:
  CallerManager();
  ~CallerManager();
public:
  bool OnResult(std::thread::id id, std::shared_ptr<IAsyncResult> result);

  bool PumpMessage();

  template<typename R>
  AsyncResultWrapper<R> NOTHROW TryBlock(AsyncResultWrapper<R>& wrapper);

  AsyncResultWrapper<void> NOTHROW TryBlock(AsyncResultWrapper<void>& wrapper) { return wrapper; }

  int NOTHROW CreateFiber(const std::function<int(void)>& f);
  void NOTHROW _OnFiberWork();
private:
  std::thread::id m_initThreadId;
  bool m_useFiber;
  size_t m_maxFiberNum;
  std::unordered_map<st_thread_t, std::shared_ptr<FunctionWrapper>> m_fiberThreadMap;

  mutable std::mutex m_resultMutex;
  std::map<std::thread::id, std::list<std::shared_ptr<IAsyncResult>>> m_specThreadResult;
};

template<typename R>
AsyncResultWrapper<R> CallerManager::TryBlock(AsyncResultWrapper<R>& wrapper)
{
  if (!m_useFiber)
    return wrapper;

  st_cond_t t = wrapper.get_fiber_cond();
  wrapper.set_fiber_callback([t]() {
    st_cond_signal(t);
  });

  return wrapper;
}

#endif  //! #ifndef YY_CALL_MANAGER_H_
