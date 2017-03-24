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

#include "connection_monitor.h"
#include "../acceptor/rpc_client_accept.h"
#include "../util/base_util.h"
#include "../transport/rpc_tcp_server_transport.h"

_START_ORPC_NAMESPACE_

TcpPeer::TcpPeer(const std::shared_ptr<RpcTcpServerTransport>& conn)
: m_conn(conn)
{

}

bool TcpPeer::SendData(BufferStream& buff)
{
  if (m_conn)
    return m_conn->SendData(buff);

  return false;
}

void TcpPeer::CloseConnection()
{
  if (m_conn)
    m_conn->CloseConnection();
}

const std::string& TcpPeer::GetPeerIp() const
{
  static std::string invalid_ip = "invalid_ip";
  if (m_conn)
    return m_conn->GetPeerIp();
  return invalid_ip;
}

int32_t TcpPeer::GetPeerPort() const
{
  if (m_conn)
    return m_conn->GetPeerPort();
  return -1;
}

TransportManager::TransportManager()
{
  m_connMonitor = nullptr;
}

TransportManager::~TransportManager()
{

}

int TransportManager::RegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn)
{
  if (!conn)
    return -1;

  std::lock_guard<std::mutex> l(m_transportMutex);
  std::shared_ptr<TcpPeer> peer = std::make_shared<TcpPeer>(conn);
  m_rsConn[(int64_t)conn.get()] = peer;

  conn->SetPeer(peer);

  if (m_connMonitor)
    m_openList.push_back(peer);
  return 0;
}

int TransportManager::UnRegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn)
{
  if (!conn)
    return -1;

  std::lock_guard<std::mutex> l(m_transportMutex);
  std::shared_ptr<TcpPeer> peer = conn->GetPeer();

  std::shared_ptr<TcpPeer> null;
  conn->SetPeer(null);
  m_rsConn.erase((int64_t)conn.get());

  if (m_connMonitor)
    m_closeList.push_back(peer);

  return 0;
}

void TransportManager::SetConnectMonitor(ConnectionMonitor* monitor)
{
  std::lock_guard<std::mutex> l(m_transportMutex);
  if (monitor)
  {
    for (auto it : m_rsConn)
      m_openList.push_back(it.second);
  }
  m_connMonitor = monitor;
}

bool TransportManager::PumpMonitorMessage()
{
  if (!m_connMonitor)
    return true;

  std::list<std::shared_ptr<TcpPeer>> openList;
  std::list<std::shared_ptr<TcpPeer>> closeList;
  {
    std::lock_guard<std::mutex> l(m_transportMutex);
    openList = std::move(m_openList);
    closeList = std::move(m_closeList);
  }

  for (auto it : openList)
    m_connMonitor->OnConnected(it.get());

  for (auto it : closeList)
    m_connMonitor->OnDisConnected(it.get());

  return true;
}

_END_ORPC_NAMESPACE_
