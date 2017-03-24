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

#ifndef ORPC_BUFFER_STREAM_H_
#define ORPC_BUFFER_STREAM_H_

#include "build_config.h"
#include <memory>
#include "packet/packet.h"
#include <string.h>

_START_ORPC_NAMESPACE_

#define BUFFER_STREAM_INIT_SIZE 1024

class BufferStream
{
public:
  BufferStream(size_t initsz = BUFFER_STREAM_INIT_SIZE);
  ~BufferStream();

  BufferStream(const BufferStream&) = delete;
  BufferStream& operator=(const BufferStream&) = delete;

  BufferStream(BufferStream&& other);

  BufferStream& operator=(BufferStream&& other);

  void write(const char* buf, size_t len);

  char* data()
  {
    return m_data + Packet::PACKET_HEAD_LENGTH;
  }

  const char* data() const
  {
    return m_data + Packet::PACKET_HEAD_LENGTH;
  }

  size_t size() const
  {
    return m_size;
  }

  char* release();
  void clear();
private:
  void expand_buffer(size_t len);
private:
  size_t m_size;
  char* m_data;
  size_t m_alloc;
};

#include "buff_stream.inl"

_END_ORPC_NAMESPACE_

#endif  //! #ifndef ORPC_BUFFER_STREAM_H_
