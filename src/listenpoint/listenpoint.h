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

#ifndef __ORPC_LISTENPOINT_H_
#define __ORPC_LISTENPOINT_H_

#include "../build_config.h"
#include <string>
#include <stdint.h>
#include <memory>
#include "../transport/transport.h"
#include "../acceptor/tcp_acceptor.h"
#include "../thread/thread_worker.h"

_START_ORPC_NAMESPACE_

enum ListenPointStatus
{
  LPS_INIT,
  LPS_LISTEN_FAIL,
  LPS_LISTEN_SUCC,
};

class ListenPoint : public ThreadWorker
{
public:
  int Init();
  int UnInit();
public:
  // ThreadWorker
  int _DoWork() override;
  int _OnAsync(uv_async_t* handle) override;
  int _OnDestory(uv_handle_t* handle) override;
public:
  const std::string& GetIp() const { return m_ip; }
  int32_t GetPort() const { return m_port; }
  ListenPointStatus GetStatus() const { return m_status; }

  virtual void OnDestory() = 0;
  virtual int BindListen(int32_t mProtocal) = 0;
  virtual int32_t GetProtocal() const = 0;
 
  int32_t GetTransportFlags() const { return m_transportFlags; }
private:
  std::string m_ip;
  int32_t m_port;
  ListenPointStatus m_status;
  int32_t m_transportFlags;
private:
  ListenPoint(const std::string& ip, int32_t port, int32_t flags);
  friend class TcpListenPoint;
};

class RpcClientAccept;

class TcpListenPoint : public ListenPoint
{
private:
  TcpListenPoint(const std::string& ip, int32_t port, int32_t flags) : ListenPoint(ip, port, flags) {}
private:
  int32_t GetProtocal() const override { return TP_TCP; }
  void OnDestory() override;
  int BindListen(int32_t flags) override;
private:
  std::shared_ptr<RpcClientAccept> m_acceptor;
  friend class ListenPointManager;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef __ORPC_LISTENPOINT_H_
