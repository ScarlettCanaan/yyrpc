#ifndef RPC_CLIENT_ACCEPT_H_
#define RPC_CLIENT_ACCEPT_H_

#include "uv.h"
#include "acceptor/tcp_acceptor.h"

class RpcClientAccept : public TcpAcceptor
{
public:
  RpcClientAccept();
  ~RpcClientAccept();
public:
  virtual TcpServerTransport* NewTcpServerTransport();
};

#endif  //! #ifndef RPC_CLIENT_ACCEPT_H_
