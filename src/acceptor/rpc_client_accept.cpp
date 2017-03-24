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

#include "rpc_client_accept.h"
#include <algorithm>
#include "../transport/rpc_tcp_server_transport.h"
#include "../util/base_util.h"
#include "../json/msgpack2json.h"
#include "../listenpoint/listenpoint_manager.h"
#include "../stub/callee_manager.h"

#define HTTP_RESPONSE_HEADER "HTTP/1.1 200 OK\r\n"\
"Server: YCS\r\n"\
"Keep-Alive: timeout=15, max=100\r\n"\
"Connection: Keep-Alive\r\n"\
"Content-Type: application/json; charset=utf-8\r\n"\
"Content-Length: "

_START_ORPC_NAMESPACE_

ResultPacket::ResultPacket(const std::weak_ptr<RpcTcpServerTransport>& conn_, BufferStream& buff_, bool isJson_)
:conn(conn_), buff(std::move(buff_)), isJsonCall(isJson_)
{
 
}

RpcClientAccept::RpcClientAccept(int32_t flags)
  : m_transportFlags(flags)
{
  m_asyncHandle = (uv_async_t*)malloc(sizeof(uv_async_t));
  m_asyncHandle->data = 0;
}

RpcClientAccept::~RpcClientAccept()
{
  if (m_asyncHandle->data == this)
    m_asyncHandle->data = 0;
  else
    free(m_asyncHandle);
}

static void async_close_cb(uv_handle_t* handle)
{
  free(handle);
}

static void sv_async_cb(uv_async_t* handle)
{
  if (handle->data == 0)
  {
    uv_close((uv_handle_t*)handle, async_close_cb);
    return;
  }

  RpcClientAccept* worker = (RpcClientAccept*)handle->data;
  worker->_OnAsync(handle);
}

int RpcClientAccept::_OnAsync(uv_async_t* handle)
{
  std::list<std::shared_ptr<ResultPacket>> result;
  m_resultList.clone(result);
  for (std::list<std::shared_ptr<ResultPacket>>::iterator it = result.begin(); it != result.end(); ++it)
    DoSend(*it);

  return 0;
}

int RpcClientAccept::DoSend(std::shared_ptr<ResultPacket>& packet)
{
  if (std::this_thread::get_id() != m_networkThreadId)
  {
    ORPC_LOG(ERROR) << "send on wrong thread.";
    return -1;
  }

  std::shared_ptr<RpcTcpServerTransport> conn = packet->conn.lock();
  if (!conn)
  {
    return -1;
  }

  if (packet->buff.size() == 0)
  {
    conn->Close(0);
    return -1;
  }

#ifdef ORPC_SUPPROT_HTTP
  if (packet->isJsonCall)
  {
    MsgPack2Json converter;
    converter.pretty = true;
    converter.in_string = std::string(packet->buff.data(), packet->buff.size());
    converter.Convert();
    packet->buff.clear();
    packet->buff.write(converter.out_string.c_str(), converter.out_string.length());
  }
#endif  //! #ifdef ORPC_SUPPROT_HTTP

  int r = -1;
  if (IS_HTTP(m_transportFlags))
  {
    std::string s = HTTP_RESPONSE_HEADER;
    s += NumberToString(packet->buff.size());
    s += "\r\n\r\n";
    s.append(packet->buff.data(), packet->buff.size());
    r = conn->Send(s.c_str(), s.length());
  }
  else
  {
    r = conn->Send(ORPC_PROTOCAL_RESULT, packet->buff);
  }
  
  return 0;
}

int RpcClientAccept::Init(const std::string& ip, int port, uv_loop_t* loop)
{
  if (TcpAcceptor::Init(ip, port, loop) != 0)
    return -1;

  m_networkThreadId = std::this_thread::get_id();

  int r;
  r = uv_async_init(loop, m_asyncHandle, sv_async_cb);
  UV_CHECK_RET_1(uv_async_init, r);
  uv_unref((uv_handle_t*)m_asyncHandle);
  m_asyncHandle->data = this;
  return 0;
}

int RpcClientAccept::UnInit()
{
  return TcpAcceptor::UnInit();
}

TcpServerTransport* RpcClientAccept::NewTcpServerTransport()
{ 
  std::shared_ptr<RpcTcpServerTransport> conn = std::make_shared<RpcTcpServerTransport>(this, m_transportFlags);
  RegisterTransport(conn);
  return conn.get();
}

int RpcClientAccept::RegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn)
{
  if (!conn)
    return -1;

  CalleeManager::GetInstance().RegisterTransport(conn);

  m_rsConn[(int64_t)conn.get()] = conn;
  return 0;
}

int RpcClientAccept::UnRegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn)
{
  if (!conn)
    return -1;

  CalleeManager::GetInstance().UnRegisterTransport(conn);
  m_rsConn.erase((int64_t)conn.get());
  return 0;
}

bool RpcClientAccept::QueueSend(const std::weak_ptr<RpcTcpServerTransport>& conn, BufferStream& buff, bool sendImmediate, bool isJsonCall)
{
  std::shared_ptr<ResultPacket> result = std::make_shared<ResultPacket>(conn, buff, isJsonCall);
  if (sendImmediate)
  {
    DoSend(result);
  }
  else
  {
    m_resultList.push_back(result);
    uv_async_send(m_asyncHandle);
  }
  return true;
}

_END_ORPC_NAMESPACE_
