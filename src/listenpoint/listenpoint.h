#ifndef __YYRPC_LISTENPOINT_H_
#define __YYRPC_LISTENPOINT_H_

#include <string>
#include <stdint.h>
#include <memory>
#include "transport/transport.h"
#include "acceptor/tcp_acceptor.h"
#include "thread/thread_worker.h"

class ListenPoint : public ThreadWorker
{
public:
  // ThreadWorker
  int _DoWork() override;
  int _OnAsync(uv_async_t* handle) override;
  int _OnClose(uv_handle_t* handle) override;

  int _OnIdle(uv_idle_t *handle);
public:
  const std::string& GetIp() const { return m_ip; }
  int32_t GetPort() const { return m_port; }
  TransportProtocol GetProtocal() const { return m_protocal; }
private:
  int BindTcpListen();
private:
  std::string m_ip;
  int32_t m_port;
  TransportProtocol m_protocal;

  std::shared_ptr<IAcceptor> m_acceptor;
private:
  ListenPoint(const std::string& ip, int32_t port, TransportProtocol protocal);
  int Init();
  int UnInit();
  friend class ListenPointManager;

};

#endif  //! #ifndef __YYRPC_LISTENPOINT_H_
