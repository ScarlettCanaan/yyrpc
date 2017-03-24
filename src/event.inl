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
inline TRpcEvent <R(Args...)>::TRpcEvent(const std::string& name, const std::string& ns, const char* funcStr)
{
  m_name = new TRpcEventName;
  m_name->m_vParamName = tokenize_param_name(funcStr);
  m_name->m_fullName = ns + name;
}

template <typename R, typename ... Args>
inline TRpcEvent <R(Args...)>::~TRpcEvent()
{
  delete m_name;
}

template <typename R, typename ... Args>
template<typename ... Args2>
CallResultWrapper<void> TRpcEvent <R(Args...)>::operator()(Args2... args) const
{
  auto wrapper = CallerManager::GetInstance().GetNameCenter();
  if (!wrapper)
    return CallResultWrapper<R>(ORPC_ERROR_CANT_FIND_METHOD);

  BufferStream s;
  uint64_t session_id = CheckAndSerialization<R(Args...)>(s, 2/*"fire"*/, m_name->m_fullName, m_name->m_vParamName, args...);
  if (session_id == -1)
    return CallResultWrapper<R>(ORPC_ERROR_MARSHAL_FAILED);

  return wrapper.endpoint->Call<void>(session_id, s);
}

template <typename R, typename ... Args>
CallResultWrapper<void> TRpcEvent <R(Args...)>::operator()() const
{
  auto wrapper = CallerManager::GetInstance().GetNameCenter();
  if (!wrapper)
    return CallResultWrapper<R>(ORPC_ERROR_ENDPOINT_INVALID);

  BufferStream s;
  uint64_t session_id = CheckAndSerialization<R(void)>(s, 2/*"fire"*/, m_name->m_fullName, m_name->m_vParamName);
  if (session_id == -1)
    return CallResultWrapper<R>(ORPC_ERROR_MARSHAL_FAILED);

  return wrapper.endpoint->Call<void>(session_id, s);
}

template <typename R, typename ... Args>
template<typename ... Args2>
BufferStream TRpcEvent <R(Args...)>::GetFireData(Args2... args) const
{
  BufferStream s;
  uint64_t session_id = CheckAndSerialization<R(Args...)>(s, 2/*"fire"*/, m_name->m_fullName, m_name->m_vParamName, args...);
  if (session_id == -1)
    s.clear();

  return std::move(s);
}

template <typename R, typename ... Args>
BufferStream TRpcEvent <R(Args...)>::GetFireData() const
{
  BufferStream s;
  uint64_t session_id = CheckAndSerialization<R(void)>(s, 2/*"fire"*/, m_name->m_fullName, m_name->m_vParamName);
  if (session_id == -1)
    s.clear();

  return std::move(s);
}

template <typename R, typename ... Args>
template <typename T>
CallResultWrapper<bool> TRpcEvent <R(Args...)>::Subscribe(const std::function<R(Args...)>& func) const
{
  CalleeManager::GetInstance().BindApi(m_name->m_fullName, func, std::is_same<T, this_thread_t>::value);
  auto wrapper = CallerManager::GetInstance().GetNameCenter();
  if (!wrapper)
    return CallResultWrapper<bool>(ORPC_ERROR_ENDPOINT_INVALID);

  BufferStream s;
  std::vector<std::string> v;
  v.push_back("eventName");
  uint64_t session_id = CheckAndSerialization<bool(std::string)>(s, 1/*"call"*/, "NameCenterAPi::SubscribeEvent", v, m_name->m_fullName);
  if (session_id == -1)
    return CallResultWrapper<bool>(ORPC_ERROR_MARSHAL_FAILED);

  return wrapper.endpoint->Call<bool>(session_id, s);
}

