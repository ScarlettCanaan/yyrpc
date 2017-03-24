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

#include "rpc_tcp_client_transport.h"
#include "../util/base_util.h"
#include "tcp_sync_client_transport.h"

_START_ORPC_NAMESPACE_

RpcTcpClientTransport::RpcTcpClientTransport(int32_t flags)
{
  if (IS_SYNC_CLIENT(flags))
    m_clientTransport = std::make_shared<TcpSyncClientTransport>(this, flags);
  else
    m_clientTransport = std::make_shared<TcpClientTransport>(this, flags);
  memset(m_procFunc, 0, sizeof(ProcFun) * ORPC_PROTOCAL_MAX);
  m_procFunc[ORPC_PROTOCAL_RESULT] = &RpcTcpClientTransport::ProcessResult;
}

RpcTcpClientTransport::~RpcTcpClientTransport()
{
  
}

int RpcTcpClientTransport::OnRecvPacket(uint32_t msgType, const char* data, int32_t len)
{
  if (msgType < _countof(m_procFunc) && m_procFunc[msgType])
    return (this->*m_procFunc[msgType])(data, len);

  ORPC_LOG(ERROR) << "unknown msg_type: " << msgType;
  return -1;
}

int RpcTcpClientTransport::OnRecvPacket(const std::string& msgType, const char* data, int32_t len)
{
  return ProcessResult(data, len);
}

int RpcTcpClientTransport::ProcessResult(const char* data, int32_t len)
{
  return OnProcessResult(data, len);
}

int RpcTcpClientTransport::Connect(const std::string& ip, int port, uv_loop_t* loop)
{
  return m_clientTransport->Connect(ip, port, loop);
}

int RpcTcpClientTransport::Send(uint16_t msg_type, const char* data, size_t len)
{
  return m_clientTransport->Send(msg_type, data, len);
}

int RpcTcpClientTransport::Send(const char* data, size_t len)
{
  return m_clientTransport->Send(data, len);
}

int RpcTcpClientTransport::Send(uint16_t msg_type, BufferStream& buff)
{
  return m_clientTransport->Send(msg_type, buff);
}

int RpcTcpClientTransport::Close(int reason)
{
  return m_clientTransport->Close(reason);
}

ConnectStatus RpcTcpClientTransport::GetConnectStatus() const
{
  return m_clientTransport->GetConnectStatus();
}

void RpcTcpClientTransport::SetConnectMonitor(IConnectionMonitor* monitor)
{
  m_clientTransport->SetConnectMonitor(monitor);
}

_END_ORPC_NAMESPACE_
