#include "rpc_client_accept.h"
#include "../transport/rpc_tcp_server_transport.h"

RpcClientAccept::RpcClientAccept()
{

}

RpcClientAccept::~RpcClientAccept()
{

}

TcpServerTransport* RpcClientAccept::NewTcpServerTransport()
{ 
  RpcTcpServerTransport* conn = new RpcTcpServerTransport();
  conn->SetConnected();
  return conn;
}
