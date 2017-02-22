#ifndef YY_RPC_H_
#define YY_RPC_H_

#include "property.h"
#include "method.h"
#include "type_traits/function_traits.h"
#include <memory>

class EndPoint;

typedef void (*OnRpcInited)(bool bSucc);

bool InitRpc(const char* cfgFile, OnRpcInited callback);
const std::shared_ptr<EndPoint>& QueryEndPoint(const std::string& api);
const std::shared_ptr<EndPoint>& CreateEndPoint(const std::string& ip, int32_t port, TransportProtocol protocal, EndPointFlag flag = EPF_DEFAULT);

#endif  //! #ifndef YY_RPC_H_
