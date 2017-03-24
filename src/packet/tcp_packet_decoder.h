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

#ifndef _TCP_PACKET_DECODER_H_
#define _TCP_PACKET_DECODER_H_

#include "../build_config.h"
#include "packet.h"

_START_ORPC_NAMESPACE_

class IPacketCallback;

class PacketDecoder
{
public:
  PacketDecoder(IPacketCallback* cb);
  virtual ~PacketDecoder() {}
public:
  virtual int  Process(const char* buf, size_t len) = 0;
  virtual char* GetWriteBuffer(size_t& len) = 0;
  virtual void Reset() = 0;
protected:
  IPacketCallback* m_cb;
};

class BinaryPacketDecoder : public PacketDecoder
{
public:
  BinaryPacketDecoder(IPacketCallback* cb);
  ~BinaryPacketDecoder();
  virtual int Process(const char* buf, size_t len) override;
  virtual char* GetWriteBuffer(size_t& len) override;
  virtual void Reset() override;
private:
  int AdjustWriteSpace();
private:
  int  m_recvReadPos;
  int  m_recvWritePos;
  char* m_recvBuffer;
  static const int BufferSize = LargePacket::MAX_PACKET_LENGTH;
};

_END_ORPC_NAMESPACE_

struct http_parser;
struct http_parser_settings;

#ifdef ORPC_SUPPROT_HTTP

_START_ORPC_NAMESPACE_

enum HttpPacketType
{
  HPT_REUQEST,
  HPT_RESPONSE,
};

class HttpPacketDecoder : public PacketDecoder
{
public:
  HttpPacketDecoder(IPacketCallback* cb, HttpPacketType packetType);
  ~HttpPacketDecoder();
  virtual int Process(const char* buf, size_t len) override;
  virtual char* GetWriteBuffer(size_t& len) override;
  virtual void Reset() override;
public:
  static int scan_http_cb_on_message_begin(http_parser * parser);
  static int scan_http_cb_on_url(http_parser * parser, const char *at, size_t length);
  static int scan_http_cb_on_header_field(http_parser * parser, const char *at, size_t length);
  static int scan_http_cb_on_header_value(http_parser * parser, const char *at, size_t length);
  static int scan_http_cb_on_headers_complete(http_parser * parser);
  static int scan_http_cb_on_body(http_parser * parser, const char *at, size_t length);
  static int scan_http_cb_on_message_complete(http_parser * parser);
private:
  int on_message_begin();
  int on_url(const char *at, size_t length);
  int on_header_field(const char *at, size_t length);
  int on_header_value(const char *at, size_t length);
  int on_headers_complete();
  int on_body(const char *at, size_t length);
  int on_message_complete();
private:
  void onHeader(const std::string& filed, const std::string& value);
  int AdjustWriteSpace();
private:
  int  m_recvReadPos;
  int  m_recvWritePos;
  char* m_recvBuffer;
  int  m_errorId;

  HttpPacketType m_packetType;
  http_parser* m_parser;
  http_parser_settings* m_httpParserSettings;
  bool m_lastWasValue;
  std::string m_url;
  std::string m_curField;
  std::string m_curValue;
  char* m_bodyBuffer;
  size_t m_bodyWritePos;

  static const int BufferSize = 1024;
};

_END_ORPC_NAMESPACE_

#endif  //! #ifdef ORPC_SUPPROT_HTTP

#endif  //! #ifndef _TCP_PACKET_DECODER_H_
