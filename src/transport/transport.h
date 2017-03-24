#ifndef _TRANSPORT__H_
#define _TRANSPORT__H_

#include <uv.h>
#include "../packet/tcp_packet_decoder.h"
#include "../packet/tcp_packet_callback.h"
#include <iosfwd>
#include "../buff_stream.h"

_START_ORPC_NAMESPACE_

enum ConnectStatus
{
  CS_INIT,
  CS_CONNECTING,
  CS_FAIL,
  CS_CONNECTED,
  CS_WORKING,
  CS_CLOSED,
};

#define TP_TCP    1
#define TP_UDP    2
#define TRANSPORT_PROTOCAL_MASK 0x0000FF00

#define IS_TCP(f)          (f & TRANSPORT_PROTOCAL_MASK) & 0x0100
#define IS_UDP(f)        (f & TRANSPORT_PROTOCAL_MASK) & 0x0200

#define MP_HTTP   1
#define MP_BINARY 2
#define MP_FORCE_CREATE 4
#define MP_SYNC_CLIENT 8
#define METHOD_PROTOCAL_MASK   0x000000FF

#define IS_HTTP(f)          (f & METHOD_PROTOCAL_MASK) & 0x01
#define IS_BINARY(f)        (f & METHOD_PROTOCAL_MASK) & 0x02
#define IS_FORCE_CREATE(f)  (f & METHOD_PROTOCAL_MASK) & 0x04
#define IS_SYNC_CLIENT(f)   (f & METHOD_PROTOCAL_MASK) & 0x08

class IConnectionMonitor
{
public:
  virtual ~IConnectionMonitor() {}
  virtual int OnConnected(bool bSucc) = 0;
  virtual int OnRequireClose(int reason) = 0;
  virtual int OnAfterClose() = 0;
  virtual bool NeedWaitResult() = 0;
};

class ITransport
{
public:
  virtual ~ITransport() {}
  virtual int Send(uint16_t msg_type, const char* data, size_t len) = 0;
  virtual int Send(const char* data, size_t len) = 0;
  virtual int Send(uint16_t msg_type, BufferStream& buff) = 0;
  virtual int Close(int reason) = 0;
  virtual ConnectStatus GetConnectStatus() const = 0;
};

class IClientTransport : public ITransport
{
public:
  virtual int Connect(const std::string& ip, int port, uv_loop_t* loop = 0) = 0;
  virtual void SetConnectMonitor(IConnectionMonitor* monitor) = 0;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef _TRANSPORT__H_
