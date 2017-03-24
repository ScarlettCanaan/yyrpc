/*
* The MIT License (MIT)
*
* Copyright (c) 2017-2018 youjing@yy.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef _ORPC_PACKET_H_
#define _ORPC_PACKET_H_

#include "../build_config.h"
#include "stdint.h"
#include "stdlib.h"
#include "../util/fixed_memory_pool.h"

_START_ORPC_NAMESPACE_

struct Packet;

extern MemoryPool<Packet, 4096 * 1024> g_packetPool;

#pragma pack(1)

struct Packet
{
public:
  static const int MAX_PACKET_LENGTH = 4096;
  static const int PACKET_HEAD_LENGTH = sizeof(uint16_t) + sizeof(uint16_t);
  static const int MAX_BODY_LENGTH = MAX_PACKET_LENGTH - PACKET_HEAD_LENGTH;

public:
  uint16_t getTotalLength() const;
  void setTotalLength(uint16_t len);

  uint16_t getMsgType() const;
  void setMsgType(uint16_t type);

  uint16_t getBodyLength() const;

  const char* getBodyData() const { return body_data; }
  int setBodyData(const char* data, size_t len);

  void Release();
private:
  uint16_t total_length;
  uint16_t msg_type;
public: //olny for static_assert
  char     body_data[MAX_BODY_LENGTH];
};

struct LargePacket
{
public:
  static const int MAX_PACKET_LENGTH = 65535;
  static const int PACKET_HEAD_LENGTH = sizeof(uint16_t) + sizeof(uint16_t);
  static const int MAX_BODY_LENGTH = MAX_PACKET_LENGTH - PACKET_HEAD_LENGTH;

public:
  uint16_t getTotalLength() const;
  void setTotalLength(uint16_t len);

  uint16_t getMsgType() const;
  void setMsgType(uint16_t type);

  uint16_t getBodyLength() const;

  const char* getBodyData() const { return body_data; }
  int setBodyData(const char* data, size_t len);
private:
  uint16_t total_length;
  uint16_t msg_type;
public: //olny for static_assert
  char     body_data[MAX_BODY_LENGTH];
};

#pragma pack()

static_assert(offsetof(Packet, body_data) == offsetof(LargePacket, body_data), "Packet and LargePacket must have the same layout!");
static_assert(sizeof(Packet) == Packet::MAX_PACKET_LENGTH, "Packet size must be Packet::MAX_PACKET_LENGTH!");
static_assert(sizeof(LargePacket) == LargePacket::MAX_PACKET_LENGTH, "LargePacket size must be Large Packet::MAX_PACKET_LENGTH!");

_END_ORPC_NAMESPACE_

#endif  //! #ifndef _ORPC_PACKET_H_
