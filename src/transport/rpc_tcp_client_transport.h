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

#ifndef RPC_TCP_CLIENT_TRANPORT_H_
#define RPC_TCP_CLIENT_TRANPORT_H_

#include "../build_config.h"
#include "uv.h"
#include "../transport/tcp_client_transport.h"
#include "protocal_def.h"
#include <memory>

_START_ORPC_NAMESPACE_

class RpcTcpClientTransport : public IPacketCallback
{
public:
  RpcTcpClientTransport(int32_t flags);
  ~RpcTcpClientTransport();
public:
  virtual int OnRecvPacket(uint32_t msgType, const char* data, int32_t len) override;
  virtual int OnRecvPacket(const std::string& msgType, const char* data, int32_t len) override;

  int Connect(const std::string& ip, int port, uv_loop_t* loop = 0);
  int Send(uint16_t msg_type, const char* data, size_t len);
  int Send(const char* data, size_t len);
  int Send(uint16_t msg_type, BufferStream& buff);
  int Close(int reason);
  ConnectStatus GetConnectStatus() const;
  void SetConnectMonitor(IConnectionMonitor* monitor);
private:
  int ProcessResult(const char* data, int32_t len);

  virtual int OnProcessResult(const char* data, int32_t len) = 0;
private:
  typedef int (RpcTcpClientTransport::*ProcFun)(const char* data, int32_t len);
  ProcFun m_procFunc[ORPC_PROTOCAL_MAX];
  std::shared_ptr<IClientTransport> m_clientTransport;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef RPC_TCP_CLIENT_TRANPORT_H_
