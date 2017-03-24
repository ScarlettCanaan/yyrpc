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

template <typename R, typename ... Args>
inline TRpcMethod <R(Args...)>::TRpcMethod(const std::string& name, const std::string& ns, const char* funcStr)
{
  m_name = new TRpcMethodName;
  m_name->m_vParamName = tokenize_param_name(funcStr);
  m_name->m_fullName = ns + name;
}

template <typename R, typename ... Args>
inline TRpcMethod <R(Args...)>::~TRpcMethod()
{
  delete m_name;
}

template <typename R, typename ... Args>
template<typename ... Args2>
CallResultWrapper<R> TRpcMethod <R(Args...)>::operator()(Args2... args) const
{
  auto wrapper = CallerManager::GetInstance().QueryEndPoint(m_name->m_fullName);
  if (!wrapper)
    return CallResultWrapper<R>(ORPC_ERROR_CANT_FIND_METHOD);

  BufferStream s;
  uint64_t session_id = CheckAndSerialization<R(Args...)>(s, 1/*"call"*/, m_name->m_fullName, m_name->m_vParamName, args...);
  if (session_id == -1)
    return CallResultWrapper<R>(ORPC_ERROR_MARSHAL_FAILED);

  auto r = wrapper.endpoint->Call<R>(session_id, s);
#ifdef ORPC_USE_FIBER 
  CallerManager::GetInstance().TryBlock(r);
#endif  //! #ifdef ORPC_USE_FIBER
  return r;
}

template <typename R, typename ... Args>
template<typename ... Args2>
CallResultWrapper<R> TRpcMethod <R(Args...)>::operator()(const EndPointWrapper& wrapper, Args2... args) const
{
  if (!wrapper)
    return CallResultWrapper<R>(ORPC_ERROR_ENDPOINT_INVALID);

  BufferStream s;
  uint64_t session_id = CheckAndSerialization<R(Args...)>(s, 1/*"call"*/, m_name->m_fullName, m_name->m_vParamName, args...);
  if (session_id == -1)
    return CallResultWrapper<R>(ORPC_ERROR_MARSHAL_FAILED);

  auto r = wrapper.endpoint->Call<R>(session_id, s);
#ifdef ORPC_USE_FIBER 
  CallerManager::GetInstance().TryBlock(r);
#endif  //! #ifdef ORPC_USE_FIBER
  return r;
}

template <typename R, typename ... Args>
CallResultWrapper<R> TRpcMethod <R(Args...)>::operator()() const
{
  auto wrapper = CallerManager::GetInstance().QueryEndPoint(m_name->m_fullName);
  if (!wrapper)
    return CallResultWrapper<R>(ORPC_ERROR_CANT_FIND_METHOD);

  BufferStream s;
  uint64_t session_id = CheckAndSerialization<R(void)>(s, 1/*"call"*/, m_name->m_fullName, m_name->m_vParamName);
  if (session_id == -1)
    return CallResultWrapper<R>(ORPC_ERROR_MARSHAL_FAILED);

  auto r = wrapper.endpoint->Call<R>(session_id, s);
#ifdef ORPC_USE_FIBER 
  CallerManager::GetInstance().TryBlock(r);
#endif  //! #ifdef ORPC_USE_FIBER
  return r;
}

template <typename R, typename ... Args>
CallResultWrapper<R> TRpcMethod <R(Args...)>::operator()(const EndPointWrapper& wrapper) const
{
  if (!wrapper)
    return CallResultWrapper<R>(ORPC_ERROR_ENDPOINT_INVALID);
  BufferStream s;
  uint64_t session_id = CheckAndSerialization<R(void)>(s, 1/*"call"*/, m_name->m_fullName, m_name->m_vParamName);
  if (session_id == -1)
    return CallResultWrapper<R>(ORPC_ERROR_MARSHAL_FAILED);
  auto r = wrapper.endpoint->Call<R>(session_id, s);
#ifdef ORPC_USE_FIBER 
  CallerManager::GetInstance().TryBlock(r);
#endif  //! #ifdef ORPC_USE_FIBER
  return r;
}

template <typename R, typename ... Args>
template <typename T>
bool TRpcMethod <R(Args...)>::Bind(const std::function<R(Args...)>& func) const
{
  return CalleeManager::GetInstance().BindApi(m_name->m_fullName, func, std::is_same<T, this_thread_t>::value);
}

template <typename R, typename ... Args>
template <typename T>
bool TRpcMethod <R(Args...)>::BindWithPeer(const std::function<R(TcpPeer*, Args...)>& func) const
{
  return CalleeManager::GetInstance().BindApi(m_name->m_fullName, func, std::is_same<T, this_thread_t>::value);
}

