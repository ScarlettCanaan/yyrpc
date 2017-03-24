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

#ifndef _FIXED_MEMORY_POOL_H_
#define _FIXED_MEMORY_POOL_H_

#include "../build_config.h"
#include <limits.h>
#include <stddef.h>
#include <mutex>

_START_ORPC_NAMESPACE_

//thread safe

template <typename T, size_t BlockSize = 4096>
class MemoryPool
{
public:
  /* Member types */
  typedef T               value_type;
  typedef T*              pointer;
  typedef const T*        const_pointer;
  typedef size_t          size_type;

  /* Member functions */
  MemoryPool() throw();
  ~MemoryPool() throw();

  pointer allocate();
  void deallocate(pointer p);

  unsigned int alloc_count();
private:
  union Slot_ {
    value_type element;
    Slot_* next;
  };

  typedef char* data_pointer_;
  typedef Slot_ slot_type_;
  typedef Slot_* slot_pointer_;

  slot_pointer_ currentBlock_;
  slot_pointer_ currentSlot_;
  slot_pointer_ lastSlot_;
  slot_pointer_ freeSlots_;

  unsigned int  allocCount_;
  unsigned int  blockCount_;

  std::mutex    m;
  size_type padPointer(data_pointer_ p, size_type align) const throw();
  void allocateBlock();
};

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type
MemoryPool<T, BlockSize>::padPointer(data_pointer_ p, size_type align)
const throw()
{
  size_t result = reinterpret_cast<size_t>(p);
  return ((align - result) % align);
}


template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool()
throw()
{
  allocCount_ = 0;
  blockCount_ = 0;
  currentBlock_ = 0;
  currentSlot_ = 0;
  lastSlot_ = 0;
  freeSlots_ = 0;
}


template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool()
throw()
{
  slot_pointer_ curr = currentBlock_;
  while (curr != 0) {
    slot_pointer_ prev = curr->next;
    operator delete(reinterpret_cast<void*>(curr));
    curr = prev;
  }
}


template <typename T, size_t BlockSize>
void
MemoryPool<T, BlockSize>::allocateBlock()
{
  ++blockCount_;
  // Allocate space for the new block and store a pointer to the previous one
  data_pointer_ newBlock = reinterpret_cast<data_pointer_>
    (operator new(BlockSize));
  reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
  currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
  // Pad block body to staisfy the alignment requirements for elements
  data_pointer_ body = newBlock + sizeof(slot_pointer_);
  size_type bodyPadding = padPointer(body, sizeof(long long));
  currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
  lastSlot_ = reinterpret_cast<slot_pointer_>
    (newBlock + BlockSize - sizeof(slot_type_) + 1);
}


template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::allocate()
{
  std::lock_guard<std::mutex> lock(m);
  ++allocCount_;
  if (freeSlots_ != 0) {
    pointer result = reinterpret_cast<pointer>(freeSlots_);
    freeSlots_ = freeSlots_->next;
    return result;
  }
  else {
    if (currentSlot_ >= lastSlot_)
      allocateBlock();
    pointer result = reinterpret_cast<pointer>(currentSlot_++);
    return result;
  }
}


template <typename T, size_t BlockSize>
inline void
MemoryPool<T, BlockSize>::deallocate(pointer p)
{
  if (p != 0) {
    std::lock_guard<std::mutex> lock(m);
    --allocCount_;
    reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
    freeSlots_ = reinterpret_cast<slot_pointer_>(p);
  }
}

template <typename T, size_t BlockSize>
inline unsigned int 
MemoryPool<T, BlockSize>::alloc_count()
{
  std::lock_guard<std::mutex> lock(m);
  unsigned int count = allocCount_;
  return count;
}

_END_ORPC_NAMESPACE_

#endif  //! #ifndef _FIXED_MEMORY_POOL_H_
