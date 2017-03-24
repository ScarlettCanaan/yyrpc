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

#include "tcp_acceptor.h"
#include <memory>
#include "../util/base_util.h"

_START_ORPC_NAMESPACE_

TcpAcceptor::TcpAcceptor()
{
  m_port = -1;
}

TcpAcceptor::~TcpAcceptor()
{

}

static void sv_connection_cb(uv_stream_t* server_handle, int status) {
  TcpAcceptor* ai = (TcpAcceptor*)server_handle->data;
  ai->_OnTcpConnection(server_handle, status);
}

static void sv_alloc_cb(uv_handle_t* handle,
  size_t suggested_size,
  uv_buf_t* buf) {
  TcpServerTransport* peer = (TcpServerTransport*)handle->data;
  size_t len = 0;
  buf->base = peer->decoder()->GetWriteBuffer(len);
  buf->len = len;
}

static void sv_read_cb(uv_stream_t* handle,
  ssize_t nread,
  const uv_buf_t* buf) {
  TcpServerTransport* peer = (TcpServerTransport*)handle->data;
  peer->OnRead(handle, nread, buf);
}

int TcpAcceptor::_OnTcpConnection(uv_stream_t* server_handle, int status)
{
  if (status < 0)
    return 0;

  int r;
  TcpServerTransport* connection (NewTcpServerTransport());
  if (!connection)
    return -1;

  uv_tcp_t* peer_t = connection->storage();
  peer_t->data = connection;

  r = uv_tcp_init(server_handle->loop, (uv_tcp_t*)peer_t);
  UV_CHECK_RET_1(uv_tcp_init, r);

  uv_tcp_nodelay(peer_t, true);

  r = uv_accept(server_handle, (uv_stream_t*)peer_t);
  UV_CHECK_RET_1(uv_accept, r);

  connection->SetConnected();

  r = uv_read_start((uv_stream_t*)peer_t, sv_alloc_cb, sv_read_cb);
  UV_CHECK_RET_1(uv_read_start, r);

  return 0;
}

int TcpAcceptor::Init(const std::string& ip, int port, uv_loop_t* loop)
{
  m_ip = ip;
  m_port = port;

  int r;
  r = uv_ip4_addr(ip.c_str(), port, &m_listenAddr);
  UV_CHECK_RET_1(uv_ip4_addr, r);

  m_tcpListener.data = (void*)this;
  r = uv_tcp_init(loop ? loop : uv_default_loop(), (uv_tcp_t*)&m_tcpListener);
  UV_CHECK_RET_1(uv_tcp_init, r);

  r = uv_tcp_bind((uv_tcp_t*)&m_tcpListener, (const struct sockaddr*)&m_listenAddr, 0);
  UV_CHECK_RET_1(uv_tcp_bind, r);

  return 0;
}

int TcpAcceptor::UnInit()
{
  uv_close((uv_handle_t*)&m_tcpListener, 0);
  return 0;
}

int TcpAcceptor::Listen()
{
  int r;

  r = uv_listen((uv_stream_t*)&m_tcpListener,
    1024,
    sv_connection_cb);
  UV_CHECK_RET_1(uv_listen, r);

  std::cout << "listen on: " << m_ip << " port:" << m_port << std::endl;

  return 0;
}

_END_ORPC_NAMESPACE_
