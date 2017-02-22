#ifndef YYRPC_COMMON_API_H
#define YYRPC_COMMON_API_H

#include "yyrpc.h"

struct SocketDisconnectMessage
{
  int32_t message_id;
  std::string message;
  YYRPC_PROPERTY(message_id, message);
};

namespace EventAPi
{

}

#endif  //! #ifndef TEST_API_H