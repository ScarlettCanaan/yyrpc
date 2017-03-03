#ifndef RPC_CLIENT_ACCEPT_H_
#define RPC_CLIENT_ACCEPT_H_

#include "uv.h"
#include "acceptor/tcp_acceptor.h"
#include <list>
#include <memory>
#include "util/thread_safe_list.h"

class RpcTcpServerTransport;

struct ResultPacket
{
  ResultPacket(const std::weak_ptr<RpcTcpServerTransport>& conn_, std::stringstream& s);
  std::weak_ptr<RpcTcpServerTransport> conn;
  std::string data;
};

class RpcClientAccept : public TcpAcceptor
{
public:
  RpcClientAccept(MethodProtocol mProtocal);
  ~RpcClientAccept();
public:
  virtual int Init(const std::string& ip, int port, uv_loop_t* loop = 0) override;
  virtual int UnInit() override;
  virtual TcpServerTransport* NewTcpServerTransport() override;
public:
  int RegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn);
  int UnRegisterTransport(const std::shared_ptr<RpcTcpServerTransport>& conn);

  int _OnAsync(uv_async_t* handle);
  bool QueueSend(const std::weak_ptr<RpcTcpServerTransport>& conn, std::stringstream& s);

private:
  int DoSend(std::shared_ptr<ResultPacket>& packet);
private:
  MethodProtocol m_methodProtocal;
  uv_async_t* m_asyncHandle;
  ThreadSafeList<std::shared_ptr<ResultPacket>> m_resultList;
  std::list<std::shared_ptr<RpcTcpServerTransport>> m_rsConn;
};

#endif  //! #ifndef RPC_CLIENT_ACCEPT_H_
