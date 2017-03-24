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

#ifndef TCP_ACCEPT_INITIALIZER_H_
#define TCP_ACCEPT_INITIALIZER_H_

#include "../build_config.h"
#include "uv.h"
#include "../transport/tcp_server_transport.h"
#include <string>
#include "acceptor.h"

_START_ORPC_NAMESPACE_

class TcpAcceptor : public IAcceptor
{
public:
  TcpAcceptor();
  ~TcpAcceptor();
public:
  virtual int Init(const std::string& ip, int port, uv_loop_t* loop = 0);
  virtual int UnInit();

  int Listen();
public:
  int _OnTcpConnection(uv_stream_t* server_handle, int status);
public: 
  virtual TcpServerTransport* NewTcpServerTransport() = 0;
private:
  sockaddr_in m_listenAddr;
  uv_tcp_t m_tcpListener;
  int m_port;
  std::string m_ip;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef TCP_ACCEPT_INITIALIZER_H_
