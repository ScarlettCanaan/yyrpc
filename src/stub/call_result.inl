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

template<typename T>
TRpcCallResult<T>::TRpcCallResult() :
m_timeoutValueMs(3000),
m_errorNo(ORPC_ERROR_UNKNOWN)
{
#ifdef ORPC_USE_FIBER  
  m_fiberCond = st_cond_new();
#endif  //! #ifdef ORPC_USE_FIBER 
  GetTimeMillSecond(&m_calltimeValueMs);
}

template<typename T>
TRpcCallResult<T>::~TRpcCallResult()
{
#ifdef ORPC_USE_FIBER  
  st_cond_destroy(m_fiberCond);
#endif  //! #ifdef ORPC_USE_FIBER
}

template<typename T>
T TRpcCallResult<T>::Wait()
{
#ifdef ORPC_USE_FIBER
  if (st_cond_timedwait(m_fiberCond, m_timeoutValueMs * 1000) < 0)
    m_errorNo = ORPC_ERROR_CALL_TIMEOUT;

  if (m_errorNo == ORPC_ERROR_SUCESS)
  {
    return m_value;
  }
  else if (m_errorNo > 0)
  {
  #ifdef WIN32
    return T();
  #else
    std::runtime_error(ErrorIdToString(m_errorNo));
  #endif // WIN32
  }
  //never reached
  return T();

#else
  if (m_errorNo == ORPC_ERROR_SUCESS)
  {
    return m_value;
  }
  else if (m_errorNo > 0)
  {
    std::runtime_error(ErrorIdToString(m_errorNo));
  }
  else
  {
    return m_promise.get_future().get();
  }

  //never reached
  return T();
#endif //! #ifdef ORPC_USE_FIBER
}

template<typename T>
bool TRpcCallResult<T>::SetResult(const msgpack::object* obj)
{
  if (m_errorNo != ORPC_ERROR_UNKNOWN)
    return false;

  Unserialization(obj, m_value);
  m_errorNo = ORPC_ERROR_SUCESS;
#ifndef ORPC_USE_FIBER 
  m_promise.set_value(m_value);
#endif  //! #ifndef ORPC_USE_FIBER 
  return true;
}

template<typename T>
bool TRpcCallResult<T>::SetException(int32_t error)
{
  if (m_errorNo != ORPC_ERROR_UNKNOWN)
    return false;

  m_errorNo = error;
#ifndef ORPC_USE_FIBER 
  m_promise.set_exception(std::make_exception_ptr(std::runtime_error(ErrorIdToString(error))));
#endif  //! #ifndef ORPC_USE_FIBER 
  return true;
}

template<typename T>
void TRpcCallResult<T>::RunCallback() const
{
#ifdef ORPC_USE_FIBER  
  if (m_errorNo == ORPC_ERROR_SUCESS && m_fiberCallback) //fiber must call first
    m_fiberCallback();
#else
  if (m_errorNo > 0 && m_exceptionCallback)
    m_exceptionCallback(m_errorNo);

  if (m_errorNo == ORPC_ERROR_SUCESS && m_resultCallback)
    m_resultCallback(m_value);
#endif  //! #ifdef ORPC_USE_FIBER
}

template<typename T>
bool TRpcCallResult<T>::IsTimeouted(int64_t curTimeMs)
{
  if (m_errorNo != ORPC_ERROR_UNKNOWN)
    return false;

  if (curTimeMs < m_calltimeValueMs ||
    curTimeMs - m_calltimeValueMs < m_timeoutValueMs)
    return false;

  SetException(ORPC_ERROR_CALL_TIMEOUT);
  return true;
}

template<typename T>
bool TRpcCallResult<T>::HasCallback() const
{
#ifdef ORPC_USE_FIBER
  return m_fiberCallback || false;
#else
  return m_exceptionCallback || m_resultCallback;
#endif  //! #ifdef ORPC_USE_FIBER
}

#ifndef ORPC_USE_FIBER
template<typename R>
template <typename T>
CallResultWrapper<R> CallResultWrapper<R>::Then(const std::function<void(const R&)>& callback)
{
  if (!m_result)
    return *this;

  if (std::is_same<T, this_thread_t>::value)
    m_result->SetCallThreadId(std::this_thread::get_id());
  m_result->SetResultCallback(callback);
  return *this;
}

template<typename R>
template <typename T>
CallResultWrapper<R> CallResultWrapper<R>::Then(const std::function<void(const R&)>& callback, const std::function<void(int32_t)>& exception)
{
  if (!m_result)
    return *this;

  if (std::is_same<T, this_thread_t>::value)
    m_result->SetCallThreadId(std::this_thread::get_id());
  m_result->SetResultCallback(callback);
  m_result->SetExceptionCallback(exception);
  return *this;
}
#endif //! #ifndef ORPC_USE_FIBER

template<typename R>
R CallResultWrapper<R>::Wait()
{
  if (!m_result)
  {
#ifdef WIN32
    return R();
#else
    throw std::runtime_error("null obj.");
#endif  //! #ifdef WIN32
  }
    
  return m_result->Wait();
}

template<typename R>
CallResultWrapper<R> CallResultWrapper<R>::SetTimeoutValue(int64_t value)
{
  if (!m_result)
    return *this;

  m_result->SetTimeoutValue(value);
  return *this;
}

template<typename R>
int32_t CallResultWrapper<R>::GetError() const
{
  if (!m_result)
    return m_errorNo;

  return m_result->GetError();
}
