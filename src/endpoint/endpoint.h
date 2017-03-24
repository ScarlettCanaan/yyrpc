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

#ifndef __ORPC_ENDPOINT_H_
#define __ORPC_ENDPOINT_H_

#include "../build_config.h"
#include <string>
#include <stdint.h>
#include <memory>
#include "../transport/transport.h"
#include "../thread/thread_worker.h"
#include "../util/thread_safe_list.h"
#include <map>
#include <mutex>
#include "../stub/call_result.h"
#include "../transport/rpc_tcp_client_transport.h"
#include "../buff_stream.h"

_START_ORPC_NAMESPACE_

class ICallResult;
class TcpClientTransport;

enum CallPacketType
{
  EPT_CALL,
  EPT_AUTH,
};

struct CallPacket
{
  CallPacketType type;
  BufferStream buff;
};

class EndPoint : public ThreadWorker
{
public:
  int Init(int32_t max_try_count = 6);
  int UnInit();
public:
  // ThreadWorker
  int _DoWork() override;
  int _OnAsync(uv_async_t* handle) override;
  int _OnDestory(uv_handle_t* handle) override;

public:
  int _OnTimerLong(uv_timer_t *handle);
  int _OnAsyncWork(uv_async_t* handle);
public:
  template<typename R>
  CallResultWrapper<R> Call(uint64_t session_id, BufferStream& buff);

  CallResultWrapper<void> Call(uint64_t session_id, BufferStream& buff);

  int Auth(BufferStream& buff);
private:
  bool CheckCallTimeout();
  bool CheckCallTimeout(int64_t cur_time, const std::shared_ptr<ICallResult>& result);
  bool ReConnectIfNecessary();
  bool OnFinalFailed();
public:
  const std::string& GetIp() const { return m_ip; }
  int32_t GetPort() const { return m_port; }

  bool IsFinalFailed() const { return m_tryCount >= m_maxTryCount; }
  bool ResetFail() { m_tryCount = 0; return true; }

  virtual int DoTask(const std::shared_ptr<CallPacket>& task) = 0;
  virtual void OnDestory() = 0;
  virtual void CreateChannel() = 0;
  virtual int32_t GetTransportProtocal() const = 0;
  virtual ConnectStatus GetConnectStatus() const = 0;
  virtual void SetAuth(int32_t appId, int32_t privilege, int64_t session_token) = 0;

  int32_t GetTransportFlags() const { return m_transportflags; }
private:
  std::string m_ip;
  int32_t m_port;
  int32_t m_transportflags;
  uv_timer_t m_timerLong;
  std::mutex m_resultMutex;

  std::map<int32_t, std::shared_ptr<ICallResult>> m_pendingResult;

  ThreadSafeList<std::shared_ptr<CallPacket>> m_taskList;

  uv_async_t* m_asyncHandle;

  int32_t  m_maxTryCount;
  int32_t  m_tryCount;
  int64_t  m_nextTryTime;
private:
  EndPoint(const std::string& ip, int32_t port, int32_t flags);
  ~EndPoint();
  friend class TcpEndPoint;
  typedef std::list<std::shared_ptr<CallPacket>> PacketList;
};

template<typename R>
CallResultWrapper<R> EndPoint::Call(uint64_t session_id, BufferStream& buff)
{
  if (session_id == -1)
    return CallResultWrapper<R>(ORPC_ERROR_MARSHAL_FAILED);

  std::shared_ptr<TRpcCallResult<R>> pResult = std::make_shared<TRpcCallResult<R>>(); {
    std::lock_guard<std::mutex> l(m_resultMutex);
    m_pendingResult[session_id] = pResult;
  }

  std::shared_ptr<CallPacket> pPacket = std::make_shared<CallPacket>();
  pPacket->type = EPT_CALL;
  pPacket->buff = std::move(buff);
  if (IS_SYNC_CLIENT(m_transportflags))
  {
    DoTask(pPacket);
  }
  else
  {
    m_taskList.push_back(pPacket);
    uv_async_send(m_asyncHandle);
  }

  return CallResultWrapper<R>(pResult);
}

class TcpEndPoint : public EndPoint, public RpcTcpClientTransport, public IConnectionMonitor
{
private:
  TcpEndPoint(const std::string& ip, int32_t port, int32_t flags);
private:
  virtual int DoTask(const std::shared_ptr<CallPacket>& task) override;
  virtual int32_t GetTransportProtocal() const override { return TP_TCP; }
  virtual void OnDestory() override;
  virtual void CreateChannel() override;
  virtual ConnectStatus GetConnectStatus() const override { return RpcTcpClientTransport::GetConnectStatus(); }
  virtual void SetAuth(int32_t appId, int32_t privilege, int64_t session_token) override;

  virtual int OnProcessResult(const char* data, int32_t len) override;

  int OnCallResult(int32_t sessionId, int32_t errorId, const  msgpack::object_kv* obj);
  int OnFireEvent(const std::string& eventName, msgpack::object_handle& o, const  msgpack::object_kv* obj);
  
  virtual int OnConnected(bool bSucc) override;
  virtual int OnRequireClose(int reason) override;
  virtual int OnAfterClose() override;
  virtual bool NeedWaitResult() override;

  bool TrySendAuth();
private:
  int32_t m_appId;
  int32_t m_privilege;
  int64_t m_sessionToken;
  friend class EndPointManager;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef __ORPC_ENDPOINT_H_
