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

inline BufferStream::BufferStream(size_t initsz) : m_size(0), m_alloc(initsz)
{
  if (initsz == 0) {
    m_data = nullptr;
  }
  else {
    m_data = (char*)::malloc(initsz);
    if (!m_data) {
      throw std::bad_alloc();
    }
  }
}

inline BufferStream::~BufferStream()
{
  ::free(m_data);
}

inline BufferStream::BufferStream(BufferStream&& other) :
m_size(other.m_size), m_data(other.m_data), m_alloc(other.m_alloc)
{
  other.m_size = other.m_alloc = 0;
  other.m_data = nullptr;
}

inline BufferStream& BufferStream::operator=(BufferStream&& other)
{
  ::free(m_data);

  m_size = other.m_size;
  m_alloc = other.m_alloc;
  m_data = other.m_data;

  other.m_size = other.m_alloc = 0;
  other.m_data = nullptr;

  return *this;
}

inline void  BufferStream::write(const char* buf, size_t len)
{
  if (m_alloc - m_size - Packet::PACKET_HEAD_LENGTH < len)
    expand_buffer(len);

  char* data = m_data + m_size + Packet::PACKET_HEAD_LENGTH;
  if (len == 1)
  {
    *data = *buf;
  }
  else if (len == 2)
  {
    *(int16_t*)data = *(int16_t*)buf;
  }
  else if (len == 4)
  {
    *(int32_t*)data = *(int32_t*)buf;
  }
  else
  {
    memcpy(data, buf, len);
  }
  m_size += len;
}


inline char* BufferStream::release()
{
  char* tmp = m_data;
  m_size = 0;
  m_data = nullptr;
  m_alloc = 0;
  return tmp;
}

inline void BufferStream::clear()
{
  m_size = 0;
}

inline void BufferStream::expand_buffer(size_t len)
{
  size_t nsize = (m_alloc > 0) ?
    m_alloc * 2 : BUFFER_STREAM_INIT_SIZE;

  while (nsize < m_size + len) {
    size_t tmp_nsize = nsize * 2;
    if (tmp_nsize <= nsize) {
      nsize = m_size + len;
      break;
    }
    nsize = tmp_nsize;
  }

  void* tmp = ::realloc(m_data, nsize);
  if (!tmp)
    throw std::bad_alloc();

  m_data = static_cast<char*>(tmp);
  m_alloc = nsize;
}

