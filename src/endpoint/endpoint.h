#ifndef __YYRPC_ENDPOINT_H_
#define __YYRPC_ENDPOINT_H_

#include <string>
#include <stdint.h>
#include <memory>
#include "transport/transport.h"
#include "thread/thread_worker.h"
#include "util/thread_safe_list.h"
#include <map>
#include <mutex>
#include "async_result.h"
#include "transport/rpc_tcp_client_transport.h"

class IAsyncResult;
class TcpClientTransport;

struct CallerPacket
{
  CallerPacket(uint64_t session_id, std::stringstream& s);

  uint32_t m_sessionId;
  std::string m_data;
};

class EndPoint : public ThreadWorker
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
  int _OnTimerLong(uv_timer_t *handle);
  int _OnAsyncWork(uv_async_t* handle);
public:
  template<typename R>
  AsyncResultWrapper<R> Call(uint64_t session_id, std::stringstream& s);

  AsyncResultWrapper<void> Call(uint64_t session_id, std::stringstream& s);
private:
  virtual int DoTask(const std::shared_ptr<CallerPacket>& task) = 0;
  int CheckCallTimeout();
  bool CheckCallTimeout(int64_t cur_time, const std::shared_ptr<IAsyncResult>& result);
  int ReConnectIfNecessary();
public:
  const std::string& GetIp() const { return m_ip; }
  int32_t GetPort() const { return m_port; }

  bool IsFinalFailed() const;
  void ResetFail();

  virtual void OnDestory() = 0;
  virtual void CreateChannel() = 0;
  virtual TransportProtocol GetTransportProtocal() const = 0;

  MethodProtocol GetMethodProtocal() const { return m_methodProtocal; }
private:
  std::string m_ip;
  int32_t m_port;
  MethodProtocol m_methodProtocal;
  uv_timer_t timer_long;
  std::mutex m_resultMutex;

  std::map<int32_t, std::shared_ptr<IAsyncResult>> m_pendingResult;

  ThreadSafeList<std::shared_ptr<CallerPacket>> m_taskList;

  uv_async_t* m_asyncHandle;

  int32_t  m_tryCount;
  int64_t  m_nextTryTime;
private:
  EndPoint(const std::string& ip, int32_t port, MethodProtocol mProtocal);
  ~EndPoint();
  friend class TcpEndPoint;
};

template<typename R>
AsyncResultWrapper<R> EndPoint::Call(uint64_t session_id, std::stringstream& s)
{
  if (session_id == -1)
    return AsyncResultWrapper <R>(YYRPC_ERROR_SERI_FAILED);

  std::shared_ptr<AsyncResult<R>> pResult = std::make_shared<AsyncResult<R>>();
  {
    std::lock_guard<std::mutex> l(m_resultMutex);
    m_pendingResult[session_id] = pResult;
  }

  std::shared_ptr<CallerPacket> pPacket = std::make_shared<CallerPacket>(session_id, s);
  m_taskList.push_back(pPacket);
  uv_async_send(m_asyncHandle);

  return AsyncResultWrapper<R>(pResult);
}

inline AsyncResultWrapper<void> EndPoint::Call(uint64_t session_id, std::stringstream& s)
{
  if (session_id == -1)
    return AsyncResultWrapper < void >(YYRPC_ERROR_SERI_FAILED);

  std::shared_ptr<CallerPacket> pPacket = std::make_shared<CallerPacket>(session_id, s);
  m_taskList.push_back(pPacket);
  uv_async_send(m_asyncHandle);
  return AsyncResultWrapper < void >(YYRPC_ERROR_SUCESS);
}

class TcpEndPoint : public EndPoint, public RpcTcpClientTransport
{
private:
  TcpEndPoint(const std::string& ip, int32_t port, MethodProtocol mProtocal) : EndPoint(ip, port, mProtocal), RpcTcpClientTransport(mProtocal) {}
private:
  TransportProtocol GetTransportProtocal() const override { return TP_TCP; }
  void OnDestory() override;
  void CreateChannel() override;

  virtual int OnProcessResult(const char* data, int32_t len) override;

  int OnCallResult(int32_t session_id, int32_t error_id, msgpack::unpacker& unp);
  int OnFireEvent(int32_t session_id, int32_t error_id, msgpack::unpacker& unp);
  
  virtual int OnConnected(bool bSucc) override;
  virtual int OnAfterClose() override;

  virtual int DoTask(const std::shared_ptr<CallerPacket>& task) override;
private:
  friend class EndPointManager;
};

#endif  //! #ifndef __YYRPC_ENDPOINT_H_
