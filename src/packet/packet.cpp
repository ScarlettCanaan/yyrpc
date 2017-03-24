#include "packet.h"
#include "memory.h"
#include "string.h"
#include <uv.h>

_START_ORPC_NAMESPACE_

uint16_t Packet::getTotalLength() const { return ntohs(total_length); }
void Packet::setTotalLength(uint16_t len) { total_length = htons(len); }

uint16_t Packet::getMsgType() const  { return ntohs(msg_type); }
void Packet::setMsgType(uint16_t type) { msg_type = htons(type); }

uint16_t Packet::getBodyLength() const { return ntohs(total_length) - PACKET_HEAD_LENGTH; }

int Packet::setBodyData(const char* data, size_t len)
{
  if (len > MAX_BODY_LENGTH)
    return -1;
  memcpy(body_data, data, len);
  return 0;
}

void Packet::Release()
{
  if (getTotalLength() > Packet::MAX_PACKET_LENGTH)
    free(this);
  else
    g_packetPool.deallocate(this);
}

int LargePacket::setBodyData(const char* data, size_t len)
{
  if (len > MAX_BODY_LENGTH)
    return -1;
  memcpy(body_data, data, len);
  return 0;
}

uint16_t LargePacket::getTotalLength() const { return ntohs(total_length); }
void LargePacket::setTotalLength(uint16_t len) { total_length = htons(len); }

uint16_t LargePacket::getMsgType() const  { return ntohs(msg_type); }
void LargePacket::setMsgType(uint16_t type) { msg_type = htons(type); }

uint16_t LargePacket::getBodyLength() const { return ntohs(total_length) - PACKET_HEAD_LENGTH; }

MemoryPool<Packet, 4096 * 1024> g_packetPool;

_END_ORPC_NAMESPACE_
