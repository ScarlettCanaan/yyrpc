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

#include "rpc_tcp_server_transport.h"
#include "../util/base_util.h"
#include <assert.h>
#include "../../proto/common_api.h"
#include "../acceptor/rpc_client_accept.h"
#include "../json/param2msgpack.h"

_START_ORPC_NAMESPACE_

RpcTcpServerTransport::RpcTcpServerTransport(RpcClientAccept* accptor, int32_t flags)
: TcpServerTransport(flags), m_accptor(accptor)
{
  m_ioThreadId = std::this_thread::get_id();
  m_peerAppId = -1;
  m_peerSessionToken = -1;
  m_peerPrivilege = -1;

  m_bJsonCall = false;
  memset(m_procFunc, 0, sizeof(ProcFun) * ORPC_PROTOCAL_MAX);
  m_procFunc[ORPC_PROTOCAL_CALL] = &RpcTcpServerTransport::ProcessCall;
  m_procFunc[ORPC_PROTOCAL_AUTH] = &RpcTcpServerTransport::ProcessAuth;
}

RpcTcpServerTransport::~RpcTcpServerTransport()
{

}

int RpcTcpServerTransport::OnRecvPacket(uint32_t msgType, const char* data, int32_t len)
{
  if (msgType < _countof(m_procFunc) && m_procFunc[msgType])
    return (this->*m_procFunc[msgType])(data, len);

  ORPC_LOG(ERROR) << "unknown msg_type: " << msgType;
  return CR_INVALID_PACKET;
}

int RpcTcpServerTransport::OnRecvPacket(const std::string& msgType, const char* data, int32_t len)
{
  static const size_t method_request_len = strlen("/method_request/");
  if (msgType == "/method_request" && m_procFunc[ORPC_PROTOCAL_CALL])
  {
    return (this->*m_procFunc[ORPC_PROTOCAL_CALL])(data, len);
  }
  if (msgType == "/auth" && m_procFunc[ORPC_PROTOCAL_AUTH])
  {
    return (this->*m_procFunc[ORPC_PROTOCAL_AUTH])(data, len);
  }
#ifdef ORPC_SUPPROT_HTTP
  else if (msgType.length() >= method_request_len && memcmp(msgType.c_str(), "/method_request/", method_request_len) == 0)
  {
    return ProcessJsonCall(msgType, data, len);
  }
#endif  //! #ifdef ORPC_SUPPROT_HTTP

  ORPC_LOG(ERROR) << "unknown msg_type: " << msgType;
  return CR_INVALID_PACKET;
}

int RpcTcpServerTransport::ProcessCall(const char* data, int32_t len)
{
  /*static int s_session_id = 1;
  {
  BufferStream s;
  msgpack::packer<BufferStream> packer(s);
  packer.pack_map(2);
  packer.pack(0);

  if (!SerializationResultHeader(packer, 3, ++s_session_id, 0))
  return -1;

  packer.pack(3);
  if (!Serialization(packer, 0))
  return -1;
  SendData(s);
  return 0;
  }*/

  std::string s(data, len);

  std::shared_ptr<msgpack::unpacker> unp = std::make_shared<msgpack::unpacker>();
  unp->reserve_buffer(s.size());
  std::stringstream input(s);
  std::size_t actual_read_size = input.readsome(unp->buffer(), s.size());
  if (actual_read_size != s.size())
    return ORPC_ERROR_HEADER_FAILED;

  unp->buffer_consumed(actual_read_size);

  int32_t session_id = 0;
  int32_t method_type;
  std::string method_name;
  msgpack::object_handle o;
  if (!unp->next(o))
  {
    ORPC_LOG(ERROR) << "msgpack format error.";
    return ORPC_ERROR_HEADER_FAILED;
  }

  if (o.get().type != msgpack::type::MAP)
    return ORPC_ERROR_HEADER_FAILED;

  msgpack::object_map v = o.get().via.map;
  if (v.size != 2)
    return ORPC_ERROR_HEADER_FAILED;

  //header
  msgpack::object_kv* o1 = v.ptr;
  if (!UnserializationCallHeader(&o1->val, session_id, method_type, method_name))
    return ORPC_ERROR_HEADER_FAILED;

  if (method_type == 1/*"call"*/)
  {
    msgpack::object_kv* o2 = v.ptr + 1;
    return OnMethodCall(o, o2, session_id, method_name);
  }
  else if (method_type == 2/*"fire"*/)
  {
    if (!CalleeManager::GetInstance().OnDispatchEvent(method_name, data, len))
      return -1;
  }
  else
  {
    return ORPC_ERROR_HEADER_FAILED;
  }

  return 0;
}

int RpcTcpServerTransport::OnMethodCall(msgpack::object_handle& o, msgpack::object_kv* o2, int32_t session_id, const std::string& method_name)
{
  /*if (o2->key.type != msgpack::type::POSITIVE_INTEGER)
    return ORPC_ERROR_HEADER_FAILED;
    int32_t key = o2->key.as<int32_t>();
    if (key != 2)
    return ORPC_ERROR_HEADER_FAILED;*/

  std::shared_ptr<msgpack::object_handle> obj = std::make_shared<msgpack::object_handle>(o2->val, std::move(o.zone()));
  CallbackInvorkParam param(obj, session_id, shared_from_this());
  return CalleeManager::GetInstance().OnCall(method_name, param);
}

int RpcTcpServerTransport::ProcessAuth(const char* data, int32_t len)
{
  std::string s(data, len);

  std::shared_ptr<msgpack::unpacker> unp = std::make_shared<msgpack::unpacker>();
  unp->reserve_buffer(s.size());
  std::stringstream input(s);
  std::size_t actual_read_size = input.readsome(unp->buffer(), s.size());
  if (actual_read_size != s.size())
    return ORPC_ERROR_HEADER_FAILED;

  unp->buffer_consumed(actual_read_size);

  int32_t session_id = 0;
  std::string method_type;
  std::string method_name;
  msgpack::object_handle o;
  if (!unp->next(o))
  {
    ORPC_LOG(ERROR) << "msgpack format error.";
    return ORPC_ERROR_HEADER_FAILED;
  }

  if (o.get().type != msgpack::type::MAP)
    return ORPC_ERROR_HEADER_FAILED;

  msgpack::object_map v = o.get().via.map;
  if (v.size != 1)
    return ORPC_ERROR_HEADER_FAILED;

  msgpack::object_kv* o1 = v.ptr;
  if (!UnserializationAuthHeader(&o1->val, m_peerAppId, m_peerSessionToken, m_peerPrivilege))
    return ORPC_ERROR_HEADER_FAILED;

  return 0;
}

#ifdef ORPC_SUPPROT_HTTP
int RpcTcpServerTransport::ProcessJsonCall(const std::string& msgType, const char* data, int32_t len)
{
  m_bJsonCall = true;

  static const size_t method_request_len = strlen("/method_request/");
  std::size_t pos = msgType.find_first_of('?');
  if (pos == std::string::npos || pos >= 64)
    return -1;

  char method_name[64] = { 0 }; 
  size_t i = method_request_len;
  for (size_t j = 0; i < pos; ++i)
  {
    const char* ch = msgType.c_str() + i;
    if (*ch != '/') {
      method_name[j++] = *ch;
      continue;
    }
    method_name[j++] = ':';
    method_name[j++] = ':';
  }

  ++i;  //skip '?'

  Param2MsgPack converter;
  converter.in_string = std::string(msgType.c_str() + i, msgType.length() - i);
  
  if (!converter.Convert())
  {
    ORPC_LOG(ERROR) << "can't convert to msgpack, msgType: " << msgType;
    return ORPC_ERROR_HEADER_FAILED;
  }

  std::shared_ptr<msgpack::unpacker> unp = std::make_shared<msgpack::unpacker>();
  unp->reserve_buffer(converter.out_string.size());
  std::stringstream input(converter.out_string);
  std::size_t actual_read_size = input.readsome(unp->buffer(), converter.out_string.size());
  if (actual_read_size != converter.out_string.size())
    return ORPC_ERROR_HEADER_FAILED;

  unp->buffer_consumed(actual_read_size);
  msgpack::object_handle o;
  if (!unp->next(o))
    return ORPC_ERROR_HEADER_FAILED;

  if (o.get().type != msgpack::type::MAP)
    return ORPC_ERROR_HEADER_FAILED;

  std::shared_ptr<msgpack::object_handle> obj = std::make_shared<msgpack::object_handle>(o.get(), std::move(o.zone()));
  CallbackInvorkParam param(obj, 0, shared_from_this());
  return CalleeManager::GetInstance().OnCall(method_name, param);
}
#endif  //! #ifdef ORPC_SUPPROT_HTTP

int RpcTcpServerTransport::OnRequireClose(int reason)
{
  if (m_connectStatu == CS_CLOSED)
    return -1;

  if (reason != 0)
  {
    BufferStream s;
    msgpack::packer<BufferStream> packer(s);
    packer.pack_map(1);
    packer.pack(0);
    if (SerializationResultHeader(packer, 3/*"result"*/, 0, reason))
      SendData(s);
  }

  return 0;
}

int RpcTcpServerTransport::OnAfterClose()
{
  m_accptor->UnRegisterTransport(shared_from_this());
  return 0;
}

bool RpcTcpServerTransport::SendData(BufferStream& buff)
{
  bool sendImmediate = std::this_thread::get_id() == m_ioThreadId;
  return m_accptor->QueueSend(shared_from_this(), buff, sendImmediate, m_bJsonCall);
}

void RpcTcpServerTransport::CloseConnection()
{
  BufferStream null;
  m_accptor->QueueSend(shared_from_this(), null, false, false);
}

bool RpcTcpServerTransport::SendError(int64_t session_id, int32_t errNo)
{
  BufferStream s;
  msgpack::packer<BufferStream> packer(s);
  packer.pack_map(1);
  packer.pack(0);
  if (!SerializationResultHeader(packer, 3/*"result"*/, session_id, errNo))
    return false;

  SendData(s);
  return true;
}

_END_ORPC_NAMESPACE_
