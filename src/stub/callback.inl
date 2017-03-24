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

inline bool CallbackWrapper::Invork(const CallbackInvorkParam& param) const
{
  if (!m_callback)
    return false;
  return m_callback->Invork(param);
}

template<typename R, typename Function, typename Tuple, std::size_t... index>
R invoke_helper(Function&& f, Tuple&& tup, __boost::index_sequence<index...>)
{
  return f(std::get<index>(std::forward<Tuple>(tup))...);
}

template <typename R, std::size_t Arity, typename Function, typename Tuple>
R invoke_with_tuple(Function&& f, Tuple&& tup)
{
  return invoke_helper<R>(std::forward<Function>(f), std::forward<Tuple>(tup), __boost::make_index_sequence<Arity>{});
}

template<typename Function, typename Tuple, std::size_t... index>
void invoke_helper(Function&& f, Tuple&& tup, __boost::index_sequence<index...>)
{
  return f(std::get<index>(std::forward<Tuple>(tup))...);
}

template <std::size_t Arity, typename Function, typename Tuple>
void invoke_with_tuple(Function&& f, Tuple&& tup)
{
  return invoke_helper(std::forward<Function>(f), std::forward<Tuple>(tup), __boost::make_index_sequence < Arity > {});
}

template<typename R, typename Function, typename Tuple, std::size_t... index>
R invoke_helper(Function&& f, TcpPeer* peer, Tuple&& tup, __boost::index_sequence<index...>)
{
  return f(peer, std::get<index>(std::forward<Tuple>(tup))...);
}

template <typename R, std::size_t Arity, typename Function, typename Tuple>
R invoke_with_tuple(Function&& f, TcpPeer* peer, Tuple&& tup)
{
  return invoke_helper<R>(std::forward<Function>(f), peer, std::forward<Tuple>(tup), __boost::make_index_sequence < Arity > {});
}

template<typename Function, typename Tuple, std::size_t... index>
void invoke_helper(Function&& f, TcpPeer* peer, Tuple&& tup, __boost::index_sequence<index...>)
{
  return f(peer, std::get<index>(std::forward<Tuple>(tup))...);
}

template <std::size_t Arity, typename Function, typename Tuple>
void invoke_with_tuple(Function&& f, TcpPeer* peer, Tuple&& tup)
{
  return invoke_helper(std::forward<Function>(f), peer, std::forward<Tuple>(tup), __boost::make_index_sequence < Arity > {});
}

template <typename R, typename ... Args>
bool TRpcCallback < R(Args...) >::Invork(const CallbackInvorkParam& param) const
{
  if (!m_func)
    return false;

  std::tuple<Args...> t;
  if (!CheckAndUnSerialization(&param.obj->get(), t))
    return false;
  R r = invoke_with_tuple<R, sizeof...(Args)>(m_func, t);

  BufferStream s;
  msgpack::packer<BufferStream> packer(s);
  packer.pack_map(2);
  packer.pack(0);
  if (!SerializationResultHeader(packer, 3/*"result"*/, param.session_id, 0))
    return false;

  packer.pack(1);
  if (!Serialization(packer, r))
    return false;

  param.transport->SendData(s);
  return true;
}

template <typename R, typename ... Args>
bool TRpcCallback < R(TcpPeer*, Args...) >::Invork(const CallbackInvorkParam& param) const
{
  if (!m_func)
    return false;

  std::tuple<Args...> t;
  if (!CheckAndUnSerialization(&param.obj->get(), t))
    return false;
  R r = invoke_with_tuple<R, sizeof...(Args)>(m_func, param.transport->GetPeer().get(), t);

  BufferStream s;
  msgpack::packer<BufferStream> packer(s);
  packer.pack_map(2);
  packer.pack(0);
  if (!SerializationResultHeader(packer, 3/*"result"*/, param.session_id, 0))
    return false;

  packer.pack(1);
  if (!Serialization(packer, r))
    return false;

  param.transport->SendData(s);
  return true;
}

template <typename ... Args>
bool TRpcCallback < void(Args...) >::Invork(const CallbackInvorkParam& param) const
{
  if (!m_func)
    return false;

  std::tuple<Args...> t;
  if (!CheckAndUnSerialization(&param.obj->get(), t))
    return false;
  invoke_with_tuple<sizeof...(Args)>(m_func, t);

  param.transport->SendError(param.session_id, ORPC_ERROR_SUCESS);
  return true;
}

template <typename ... Args>
bool TRpcCallback < void(TcpPeer*, Args...) >::Invork(const CallbackInvorkParam& param) const
{
  if (!m_func)
    return false;

  std::tuple<Args...> t;
  if (!CheckAndUnSerialization(&param.obj->get(), t))
    return false;
  invoke_with_tuple<sizeof...(Args)>(m_func, param.transport->GetPeer().get(), t);

  param.transport->SendError(param.session_id, ORPC_ERROR_SUCESS);
  return true;
}

template <typename R>
bool TRpcCallback<R>::Invork(const CallbackInvorkParam& param) const
{
  R r = m_func();

  BufferStream s;
  msgpack::packer<BufferStream> packer(s);
  packer.pack_map(2);
  packer.pack(0);

  if (!SerializationResultHeader(packer, 3/*"result"*/, param.session_id, 0))
    return false;

  packer.pack(1);
  if (!Serialization(packer, r))
    return false;
  param.transport->SendData(s);
  return true;
}

template <typename R>
bool TRpcCallback < R(TcpPeer*) >::Invork(const CallbackInvorkParam& param) const
{
  R r = m_func(param.transport->GetPeer().get());

  BufferStream s;
  msgpack::packer<BufferStream> packer(s);
  packer.pack_map(2);
  packer.pack(0);

  if (!SerializationResultHeader(packer, 3/*"result"*/, param.session_id, 0))
    return false;

  packer.pack(1);
  if (!Serialization(packer, r))
    return false;
  param.transport->SendData(s);
  return true;
}
