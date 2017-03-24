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

#ifndef __ORPC_CONNECTION_MONITOR_H_
#define __ORPC_CONNECTION_MONITOR_H_

#include "../build_config.h"
#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <mutex>
#include <unordered_map>
#include "../buff_stream.h"

_START_ORPC_NAMESPACE_

class RpcTcpServerTransport;

class TcpPeer
{
public:
  TcpPeer(const std::shared_ptr<RpcTcpServerTransport>& conn);
  bool SendData(BufferStream& buff);
  void CloseConnection();
  const std::string& GetPeerIp() const;
  int32_t GetPeerPort() const;
private:
  std::shared_ptr<RpcTcpServerTransport> m_conn;
};

class ConnectionMonitor
{
public:
  virtual ~ConnectionMonitor() { }
public:
  virtual bool OnConnected(TcpPeer* peer) = 0;
  virtual bool OnDisConnected(TcpPeer* peer) = 0;
};

class TransportManager
{
public:
  TransportManager();
  ~TransportManager();
public:
  void SetConnectMonitor(ConnectionMonitor* monitor);

  bool PumpMonitorMessage();

  int RegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn);
  int UnRegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn);
private:
  ConnectionMonitor* m_connMonitor;

  std::unordered_map<int64_t, std::shared_ptr<TcpPeer>> m_rsConn;
  std::list<std::shared_ptr<TcpPeer>> m_openList;
  std::list<std::shared_ptr<TcpPeer>> m_closeList;
  mutable std::mutex m_transportMutex;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef __ORPC_CONNECTION_MONITOR_H_
