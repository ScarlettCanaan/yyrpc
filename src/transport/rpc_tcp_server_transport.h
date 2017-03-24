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

#ifndef RPC_TCP_SERVER_TRANSPORT_H_
#define RPC_TCP_SERVER_TRANSPORT_H_

#include "../build_config.h"
#include "uv.h"
#include "../transport/tcp_server_transport.h"
#include "protocal_def.h"
#include <memory>
#include <thread>
#include "../buff_stream.h"

_START_ORPC_NAMESPACE_

class TcpPeer;
class RpcClientAccept;

class RpcTcpServerTransport : public TcpServerTransport, public std::enable_shared_from_this<RpcTcpServerTransport>
{
public:
  RpcTcpServerTransport(RpcClientAccept* accptor, int32_t flags);
  ~RpcTcpServerTransport();
public:
  virtual int OnRecvPacket(uint32_t msgType, const char* data, int32_t len) override;
  virtual int OnRecvPacket(const std::string& msgType, const char* data, int32_t len) override;
  virtual int OnRequireClose(int reason) override;
  virtual int OnAfterClose() override;

  bool SendData(BufferStream& buff);

  bool SendError(int64_t session_id, int32_t errNo);

  void CloseConnection();

  void SetPeer(std::shared_ptr<TcpPeer>& peer) { m_peer = peer; }
  const std::shared_ptr<TcpPeer>& GetPeer() const { return m_peer; }

  int32_t GetPeerId() const           { return m_peerAppId; }
  int64_t GetPeerSessionToken() const { return m_peerSessionToken; }
  int32_t GetPeerPrivilege() const    { return m_peerPrivilege; }
private:
  int ProcessCall(const char* data, int32_t len);
  int OnMethodCall(msgpack::object_handle& o, msgpack::object_kv* o2, int32_t session_id, const std::string& method_name);
  int ProcessAuth(const char* data, int32_t len);
#ifdef ORPC_SUPPROT_HTTP
  int ProcessJsonCall(const std::string& msgType, const char* data, int32_t len);
#endif  //! #ifdef ORPC_SUPPROT_HTTP
private:
  std::thread::id m_ioThreadId;
  int32_t m_peerAppId;
  int64_t m_peerSessionToken;
  int32_t m_peerPrivilege;

  std::shared_ptr<TcpPeer> m_peer;
  bool m_bJsonCall;
  RpcClientAccept* m_accptor;
  typedef int (RpcTcpServerTransport::*ProcFun)(const char* data, int32_t len);
  ProcFun m_procFunc[ORPC_PROTOCAL_MAX];
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef RPC_TCP_SERVER_TRANSPORT_H_
