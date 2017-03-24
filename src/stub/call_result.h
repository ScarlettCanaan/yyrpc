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

#ifndef ORPC_RESULT_H_
#define ORPC_RESULT_H_

#include "../build_config.h"
#include <unordered_map>
#include <future>
#include <exception>
#include "../util/base_util.h"
#include "../error_def.h"

_START_ORPC_NAMESPACE_

struct this_thread_t
{};

struct any_thread_t
{};

class ICallResult
{
public:
  virtual ~ICallResult() {}
public:
  virtual bool SetResult(const msgpack::object* obj) = 0;
  virtual bool SetException(int32_t error) = 0;
public:
  virtual bool HasCallback() const = 0;
  virtual void RunCallback() const = 0;
  virtual bool IsTimeouted(int64_t curTimeMs) = 0;
  virtual std::thread::id GetRunThreadId() const = 0;
};

template<typename T>
class TRpcCallResult : public ICallResult
{
public:
  TRpcCallResult();
  ~TRpcCallResult();
public:
  virtual bool SetResult(const msgpack::object* obj) override;
  virtual bool SetException(int32_t error) override;
public:
  virtual bool HasCallback() const override;
  virtual void RunCallback() const override;
  virtual bool IsTimeouted(int64_t curTimeMs) override;
  virtual std::thread::id GetRunThreadId() const override { return m_callThreadId; };

public: // only for CallResultWrapper
  T NOTHROW Wait();

  void SetCallThreadId(std::thread::id id) { m_callThreadId = id; }
  
#ifdef ORPC_USE_FIBER  
  st_cond_t NOTHROW GetFiberCond() { return m_fiberCond; }
  void SetFiberCallback(const std::function<void(void)>& callback)        { m_fiberCallback = callback; }
#else
  void SetResultCallback(const std::function<void(const T&)>& callback)   { m_resultCallback = callback; }
  void SetExceptionCallback(const std::function<void(int32_t)>& callback) { m_exceptionCallback = callback; }
#endif  //! #ifdef ORPC_USE_FIBER

  int32_t GetError() const { return m_errorNo; }
  void SetTimeoutValue(int64_t value) { m_timeoutValueMs = value; }
protected:
  int32_t m_errorNo;
  int64_t m_calltimeValueMs;
  int64_t m_timeoutValueMs;
  T  m_value;
  
  std::thread::id m_callThreadId;

#ifdef ORPC_USE_FIBER
  st_cond_t m_fiberCond;
  std::function<void(void)> m_fiberCallback;
#else
  mutable std::promise<T> m_promise;
  std::function<void(const T&)> m_resultCallback;
  std::function<void(int32_t)> m_exceptionCallback;
#endif  //! #ifdef ORPC_USE_FIBER
};

template<>
class TRpcCallResult<void> : public ICallResult
{
public:
  virtual bool SetResult(const msgpack::object* obj) override { return true; };
  virtual bool SetException(int32_t error) override { return true; }
public:
  virtual bool HasCallback() const override { return false; }
  virtual void RunCallback() const override {}
  virtual bool IsTimeouted(int64_t cur_time) override { return true; };
  virtual std::thread::id GetRunThreadId() const override { return std::thread::id(); };
};

template<typename R>
struct CallResultWrapper
{
public:
  CallResultWrapper(int errorNo) : m_errorNo(errorNo) {}
  CallResultWrapper(const std::shared_ptr<TRpcCallResult<R>>& r) : m_result(r), m_errorNo(ORPC_ERROR_UNKNOWN) {}
public:
#ifndef ORPC_USE_FIBER
  template <typename T = this_thread_t>
  CallResultWrapper<R> Then(const std::function<void(const R&)>& callback);

  template <typename T = this_thread_t>
  CallResultWrapper<R> Then(const std::function<void(const R&)>& callback, const std::function<void(int32_t)>& timeout);
#endif  //! #ifndef ORPC_USE_FIBER
public:
  R Wait();

  operator bool() const { return m_result.get() != 0; }
  operator R () { return Wait(); }

#ifndef WIN32
  // in gcc , R == std::string ambiguous overload for ¡®operator=¡¯ 
  operator std::string () { return Wait(); }
#endif

  CallResultWrapper<R> SetTimeoutValue(int64_t value);

  int32_t GetError() const;
private:
  friend class FiberCallerManager;
  std::shared_ptr<TRpcCallResult<R>> m_result;
  int32_t m_errorNo;
};

template<>
struct CallResultWrapper<void>
{
public:
  CallResultWrapper(int errorNo) : m_errorNo(errorNo) {}
  CallResultWrapper(const std::shared_ptr<TRpcCallResult<void>>& r) : m_errorNo(ORPC_ERROR_SUCESS) {}
public:
  int32_t GetError() const { return m_errorNo; }
private:
  int32_t m_errorNo;
};

#include "call_result.inl"

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_RESULT_H_
