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

#include "uv.h"
#include "tcp_packet_decoder.h"
#include "tcp_packet_callback.h"
#include <assert.h>
#include <string.h>

_START_ORPC_NAMESPACE_

PacketDecoder::PacketDecoder(IPacketCallback* cb)
  : m_cb(cb)
{
  
}

BinaryPacketDecoder::BinaryPacketDecoder(IPacketCallback* cb) : PacketDecoder(cb)
{
  m_recvReadPos = 0;
  m_recvWritePos = 0;
  m_recvBuffer = new char[BufferSize * 2];
}

BinaryPacketDecoder::~BinaryPacketDecoder()
{
  delete[] m_recvBuffer;
}

void BinaryPacketDecoder::Reset()
{
  m_recvReadPos = 0;
  m_recvWritePos = 0;
}

char* BinaryPacketDecoder::GetWriteBuffer(size_t& len)
{
  len = (m_recvWritePos > BufferSize) ? 0 : BufferSize;

  if (len == 0)
    ORPC_LOG(ERROR) << "len force set to zero, m_recvWritePos : " << m_recvWritePos;

  return m_recvBuffer + m_recvWritePos;
}

int BinaryPacketDecoder::Process(const char* buf, size_t len)
{
  if (!m_cb)
  {
    ORPC_LOG(ERROR) << "m_cb is null!";
    return 0;
  }

  if (len > BufferSize)
  {
    ORPC_LOG(ERROR) << "len > BufferSize:" << BufferSize;;
    return -1;
  }

  if (m_recvWritePos > BufferSize)
  {
    ORPC_LOG(ERROR) << "m_recvWritePos > BufferSize:" << BufferSize;;
    return -1;
  }

  m_recvWritePos += len;
  while (m_recvWritePos - m_recvReadPos > sizeof(uint16_t))
  {
    uint16_t total_length = ntohs(*(uint16_t*)(m_recvBuffer + m_recvReadPos));
    if (m_recvWritePos - m_recvReadPos >= total_length)
    {
      uint16_t msg_type = ntohs(*(uint16_t*)(m_recvBuffer + m_recvReadPos + sizeof(uint16_t)));
      int r = m_cb->OnRecvPacket(msg_type, m_recvBuffer + m_recvReadPos + Packet::PACKET_HEAD_LENGTH, total_length - Packet::PACKET_HEAD_LENGTH);
      if (r != 0)
        return r;
      m_recvReadPos += total_length;
    }
    else
    {
      break;
    }
  }

  AdjustWriteSpace();

  return 0;
}

int BinaryPacketDecoder::AdjustWriteSpace()
{
  if (m_recvWritePos < BufferSize)
    return 0;

  if (m_recvReadPos == m_recvWritePos)
  {
    m_recvReadPos = 0;
    m_recvWritePos = 0;
  }
  else
  {
    memmove(m_recvBuffer, m_recvBuffer + m_recvReadPos, m_recvWritePos - m_recvReadPos);
    int preReadPos = m_recvReadPos;
    m_recvReadPos = 0;
    m_recvWritePos -= preReadPos;
  }
  
  return 0;
}

_END_ORPC_NAMESPACE_

#ifdef ORPC_SUPPROT_HTTP

_START_ORPC_NAMESPACE_

HttpPacketDecoder::HttpPacketDecoder(IPacketCallback* cb, HttpPacketType packetType)
  : PacketDecoder(cb), m_packetType(packetType)
{
  m_recvReadPos = 0;
  m_recvWritePos = 0;
  m_recvBuffer = new char[BufferSize * 2];

  m_errorId = 0;
  m_bodyBuffer = (char*)malloc(1024 * 1024);
  m_bodyWritePos = 0;

  m_parser = (http_parser*)malloc(sizeof(http_parser));
  if (m_packetType == HPT_REUQEST)
    http_parser_init(m_parser, HTTP_REQUEST);
  else 
    http_parser_init(m_parser, HTTP_RESPONSE);
  m_parser->data = this;

  m_httpParserSettings = (http_parser_settings*)malloc(sizeof(http_parser_settings));
  memset(m_httpParserSettings, 0, sizeof(http_parser_settings));
  m_httpParserSettings->on_message_begin = HttpPacketDecoder::scan_http_cb_on_message_begin;
  m_httpParserSettings->on_url = HttpPacketDecoder::scan_http_cb_on_url;
  m_httpParserSettings->on_header_field = HttpPacketDecoder::scan_http_cb_on_header_field;
  m_httpParserSettings->on_header_value = HttpPacketDecoder::scan_http_cb_on_header_value;
  m_httpParserSettings->on_headers_complete = HttpPacketDecoder::scan_http_cb_on_headers_complete;
  m_httpParserSettings->on_body = HttpPacketDecoder::scan_http_cb_on_body;
  m_httpParserSettings->on_message_complete = HttpPacketDecoder::scan_http_cb_on_message_complete;
}

HttpPacketDecoder::~HttpPacketDecoder()
{
  delete[] m_recvBuffer;

  free(m_parser);
  free(m_bodyBuffer);
  free(m_httpParserSettings);
}

void HttpPacketDecoder::Reset()
{
  m_recvReadPos = 0;
  m_recvWritePos = 0;

  m_errorId = 0;
  free(m_parser);

  m_parser = (http_parser*)malloc(sizeof(http_parser));
  if (m_packetType == HPT_REUQEST)
    http_parser_init(m_parser, HTTP_REQUEST);
  else
    http_parser_init(m_parser, HTTP_RESPONSE);
  m_parser->data = this;
}

char* HttpPacketDecoder::GetWriteBuffer(size_t& len)
{
  len = (m_recvWritePos > BufferSize) ? 0 : BufferSize;

  if (len == 0)
    ORPC_LOG(ERROR) << "len force set to zero, m_recvWritePos : " << m_recvWritePos;

  return m_recvBuffer + m_recvWritePos;
}

int HttpPacketDecoder::scan_http_cb_on_message_begin(http_parser * parser)
{ 
  return reinterpret_cast<HttpPacketDecoder*>(parser->data)->on_message_begin();
}

int HttpPacketDecoder::scan_http_cb_on_url(http_parser * parser, const char *at, size_t length)
{ 
  return reinterpret_cast<HttpPacketDecoder*>(parser->data)->on_url(at, length);
}

int HttpPacketDecoder::scan_http_cb_on_header_field(http_parser * parser, const char *at, size_t length)
{ 
  return reinterpret_cast<HttpPacketDecoder*>(parser->data)->on_header_field(at, length);
}

int HttpPacketDecoder::scan_http_cb_on_header_value(http_parser * parser, const char *at, size_t length)
{ 
  return reinterpret_cast<HttpPacketDecoder*>(parser->data)->on_header_value(at, length);
}
int HttpPacketDecoder::scan_http_cb_on_headers_complete(http_parser * parser)
{ 
  return reinterpret_cast<HttpPacketDecoder*>(parser->data)->on_headers_complete();
}

int HttpPacketDecoder::scan_http_cb_on_body(http_parser * parser, const char *at, size_t length)
{ 
  return reinterpret_cast<HttpPacketDecoder*>(parser->data)->on_body(at, length);
}

int HttpPacketDecoder::scan_http_cb_on_message_complete(http_parser * parser)
{ 
  return reinterpret_cast<HttpPacketDecoder*>(parser->data)->on_message_complete();
}

int HttpPacketDecoder::on_message_begin()
{
  m_bodyWritePos = 0;
  m_curField.clear();
  m_curValue.clear();
  m_url.clear();
  m_lastWasValue = true;
  return 0;
}

int HttpPacketDecoder::on_url(const char *at, size_t length)
{
  m_url.append(at, length);
  return 0;
}

int HttpPacketDecoder::on_header_field(const char *at, size_t length)
{
  if (m_lastWasValue)
  {
    if (!m_curField.empty())
      onHeader(m_curField, m_curValue);
    m_curField.clear();
    m_curValue.clear();
  }

  m_curField.append(at, length);
  m_lastWasValue = 0;
  return 0;
}

int HttpPacketDecoder::on_header_value(const char *at, size_t length)
{
  m_curValue.append(at, length);
  m_lastWasValue = 1;
  return 0;
}

int HttpPacketDecoder::on_headers_complete()
{
  if (!m_curField.empty())
    onHeader(m_curField, m_curValue);
  return 0;
}

void HttpPacketDecoder::onHeader(const std::string& filed, const std::string& value)
{

}

int HttpPacketDecoder::on_body(const char *at, size_t length)
{
  if (m_bodyWritePos + length >= 1024 * 1024)
    return -1;
  memcpy(m_bodyBuffer + m_bodyWritePos, at, length);
  m_bodyWritePos += length;
  return 0;
}

int HttpPacketDecoder::on_message_complete()
{
  if (m_cb)
    m_errorId = m_cb->OnRecvPacket(m_url, m_bodyBuffer, m_bodyWritePos);

  return m_errorId;
}

int HttpPacketDecoder::Process(const char* buf, size_t len)
{
  m_recvWritePos += len;
  size_t parsed = http_parser_execute(m_parser, m_httpParserSettings, buf, len);
  if (m_parser->upgrade)
  {
    //handle new protocol
  }
  else if (parsed != len)
  {
    return -1;
  }

  m_recvReadPos += parsed;

  AdjustWriteSpace();

  return m_errorId;
}

int HttpPacketDecoder::AdjustWriteSpace()
{
  if (m_recvWritePos < BufferSize)
    return 0;

  if (m_recvReadPos == m_recvWritePos)
  {
    m_recvReadPos = 0;
    m_recvWritePos = 0;
  }
  else
  {
    memmove(m_recvBuffer, m_recvBuffer + m_recvReadPos, m_recvWritePos - m_recvReadPos);
    int preReadPos = m_recvReadPos;
    m_recvReadPos = 0;
    m_recvWritePos -= preReadPos;
  }

  return 0;
}

_END_ORPC_NAMESPACE_

#endif  //! #ifdef ORPC_SUPPROT_HTTP

