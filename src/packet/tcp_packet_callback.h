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

#ifndef _TCP_PACKET_CALLBACK_H_
#define _TCP_PACKET_CALLBACK_H_

#include "packet.h"
#include "uv.h"
#include "../build_config.h"

_START_ORPC_NAMESPACE_

struct Packet;

enum CloseReason
{
  CR_PEER_CLOSED = 0,  //peer closed
  CR_INVALID_PACKET,   // invalid packet
  CR_FORCE_CLEAN,
  CR_COMMON_MAX,
};

struct TcpWriter
{
  uv_write_t write;
  void* conn;
  Packet* packet;
  const char* data;
  TcpWriter(void* v, Packet* p, const char* d) : conn(v), packet(p), data(d) { }
  void Release();
};

inline void TcpWriter::Release()
{
  if (packet)
    packet->Release();
  if (data)
    free((void*)data);
  delete this;
}

class IPacketCallback
{
public:
  virtual int OnRecvPacket(uint32_t msgType, const char* data, int32_t len) = 0;
  virtual int OnRecvPacket(const std::string& msgType, const char* data, int32_t len) = 0;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef _TCP_PACKET_CALLBACK_H_
