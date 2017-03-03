#ifndef __YYRPC_LISTENPOINT_H_
#define __YYRPC_LISTENPOINT_H_

#include <string>
#include <stdint.h>
#include <memory>
#include "transport/transport.h"
#include "acceptor/tcp_acceptor.h"
#include "thread/thread_worker.h"

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
  virtual int BindListen(MethodProtocol mProtocal) = 0;
  virtual TransportProtocol GetProtocal() const = 0;

  MethodProtocol GetMethodProtocal() const { return m_methodProtocal; }
private:
  std::string m_ip;
  int32_t m_port;
  ListenPointStatus m_status;
  MethodProtocol m_methodProtocal;
private:
  ListenPoint(const std::string& ip, int32_t port, MethodProtocol mProtocal);
  friend class TcpListenPoint;
};

class TcpListenPoint : public ListenPoint
{
private:
  TcpListenPoint(const std::string& ip, int32_t port, MethodProtocol mProtocal) : ListenPoint(ip, port, mProtocal) {}
private:
  TransportProtocol GetProtocal() const override { return TP_TCP; }
  void OnDestory();
  int BindListen(MethodProtocol mProtocal) override;
private:
  std::shared_ptr<IAcceptor> m_acceptor;
  friend class ListenPointManager;
};

#endif  //! #ifndef __YYRPC_LISTENPOINT_H_
