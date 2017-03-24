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

#include "listenpoint.h"
#include "../acceptor/rpc_client_accept.h"
#include "../util/base_util.h"

_START_ORPC_NAMESPACE_

ListenPoint::ListenPoint(const std::string& ip, int32_t port, int32_t flags)
: m_ip(ip), m_port(port), m_transportFlags(flags)
{
  m_status = LPS_INIT;
}

int ListenPoint::Init()
{
  Start();

  return 0;
}

int ListenPoint::UnInit()
{
  Stop();
  Join();

  return 0;
}

int ListenPoint::_DoWork()
{
  _PrepareWork();

  int r;

  r = BindListen(m_transportFlags);
  m_status = (r == 0 ? LPS_LISTEN_SUCC : LPS_LISTEN_FAIL);
  UV_CHECK_RET_1(BindListen, r);

  r = uv_run(&m_loop, UV_RUN_DEFAULT);
  UV_CHECK_RET_1(uv_run, r);

  r = uv_loop_close(&m_loop);
  UV_CHECK_RET_1(uv_loop_close, r);

  return 0;
}

int ListenPoint::_OnAsync(uv_async_t* handle)
{
  ThreadWorker::_OnAsync(handle);
  return 0;
}

int ListenPoint::_OnDestory(uv_handle_t* handle)
{
  OnDestory();
  return 0;
}

int TcpListenPoint::BindListen(int32_t flags)
{
  m_acceptor = std::make_shared<RpcClientAccept>(flags);
  if (m_acceptor->Init(m_ip, m_port, &m_loop) != 0)
    return -1;
 
  return m_acceptor->Listen();
}

void TcpListenPoint::OnDestory()
{
  m_acceptor.reset();
}

_END_ORPC_NAMESPACE_
