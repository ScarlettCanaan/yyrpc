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

#ifndef RPC_CLIENT_ACCEPT_H_
#define RPC_CLIENT_ACCEPT_H_

#include "../build_config.h"
#include "uv.h"
#include "../acceptor/tcp_acceptor.h"
#include <list>
#include <memory>
#include <thread>
#include "../util/thread_safe_list.h"
#include <unordered_map>
#include "../buff_stream.h"

_START_ORPC_NAMESPACE_

class RpcTcpServerTransport;

struct ResultPacket
{
  ResultPacket(const std::weak_ptr<RpcTcpServerTransport>& conn_, BufferStream& buff_, bool isJson_);
  std::weak_ptr<RpcTcpServerTransport> conn;
  BufferStream buff;
  bool isJsonCall;
};

class RpcClientAccept : public TcpAcceptor
{
public:
  RpcClientAccept(int32_t flags);
  ~RpcClientAccept();
public:
  virtual int Init(const std::string& ip, int port, uv_loop_t* loop = 0) override;
  virtual int UnInit() override;
  virtual TcpServerTransport* NewTcpServerTransport() override;
public:
  int RegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn);
  int UnRegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn);

  int _OnAsync(uv_async_t* handle);
  bool QueueSend(const std::weak_ptr<RpcTcpServerTransport>& conn, BufferStream& buff, bool sendImmediate, bool isJsonCall);

private:
  int DoSend(std::shared_ptr<ResultPacket>& packet);
private:
  std::thread::id m_networkThreadId;
  int32_t m_transportFlags;
  uv_async_t* m_asyncHandle;
  ThreadSafeList<std::shared_ptr<ResultPacket>> m_resultList;
  std::unordered_map<int64_t, std::shared_ptr<RpcTcpServerTransport>> m_rsConn;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef RPC_CLIENT_ACCEPT_H_
