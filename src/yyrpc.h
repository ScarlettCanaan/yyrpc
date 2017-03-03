#ifndef YY_RPC_H_
#define YY_RPC_H_

#include "property.h"
#include "method.h"
#include "type_traits/function_traits.h"
#include <memory>
#include "build_config.h"

bool NOTHROW InitRpc(const char* cfgFile);
bool RunRpc();
bool UnInitRpc();

EndPointWrapper QueryEndPoint(const std::string& api);
EndPointWrapper CreateEndPoint(const std::string& ip, int32_t port, TransportProtocol tProtocal = TP_TCP, MethodProtocol mProtocal = MP_HTTP);

int NOTHROW CreateClientFiber(const std::function<int(void)>& f);

#endif  //! #ifndef YY_RPC_H_
