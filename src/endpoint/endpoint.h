#ifndef __YYRPC_ENDPOINT_H_
#define __YYRPC_ENDPOINT_H_

#include <string>
#include <stdint.h>
#include <memory>
#include "transport/transport.h"

enum EndPointFlag
{
  EPF_DEFAULT = 0x0,
  EPF_PREPARE_CHANNEL = 0x0001,
};

class EndPoint
{
public:
  const std::string& GetIp() const { return m_ip; }
  int32_t GetPort() const { return m_port; }
  TransportProtocol GetProtocal() const { return m_protocal; }

  const std::shared_ptr<ITransport>& GetChannel() const;
private:
  void CreateChannel();
private:
  std::string m_ip;
  int32_t m_port;
  TransportProtocol m_protocal;
  EndPointFlag m_flag;

  std::shared_ptr<ITransport> m_channel;
private:
  EndPoint(const std::string& ip, int32_t port, TransportProtocol protocal, EndPointFlag flag);
  friend class EndPointManager;
};

#endif  //! #ifndef __YYRPC_ENDPOINT_H_
