#include "endpoint.h"

EndPoint::EndPoint(const std::string& ip, int32_t port, TransportProtocol protocal, EndPointFlag flag)
  : m_ip(ip), m_port(port), m_protocal(protocal), m_flag(flag)
{
  if (flag & EPF_PREPARE_CHANNEL)
    CreateChannel();
}

const std::shared_ptr<ITransport>& EndPoint::GetChannel() const
{
  return m_channel;
}

void EndPoint::CreateChannel()
{
  /*if (m_protocal == TP_TCP)
    m_channel = std::make_shared<TcpChannel>(m_ip, m_port);*/
}
