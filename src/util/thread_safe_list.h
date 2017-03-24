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

#ifndef _ORPC_THREAD_SAFE_LIST_H_
#define _ORPC_THREAD_SAFE_LIST_H_

#include "../build_config.h"
#include <list>
#include <mutex>
#include <condition_variable>

#undef max

_START_ORPC_NAMESPACE_

// A threadsafe-queue.
template <class T>
class ThreadSafeList
{
public:
  ThreadSafeList(void) {}
  ~ThreadSafeList(void) {}
public:
  void push_back(T t);
  void push_front(T t);
  bool try_pop_front(T& t, std::chrono::milliseconds timeout = std::chrono::milliseconds::duration::max());
  bool try_pop_back(T& t, std::chrono::milliseconds timeout = std::chrono::milliseconds::duration::max());
  bool try_wait(std::chrono::milliseconds timeout = std::chrono::milliseconds::duration::max());
  bool clone(std::list<T>& t);
private:
  std::list<T> l;
  mutable std::mutex m;
  std::condition_variable c;
};

template <class T>
void ThreadSafeList<T>::push_back(T t)
{
  { std::lock_guard<std::mutex> lock(m); l.push_back(t); }
  c.notify_one();
}

template <class T>
void ThreadSafeList<T>::push_front(T t)
{
  { std::lock_guard<std::mutex> lock(m); l.push_front(t); }
  c.notify_one();
}

template <class T>
bool ThreadSafeList<T>::try_pop_back(T& t, std::chrono::milliseconds timeout)
{
  std::unique_lock<std::mutex> lock(m);
  if (!c.wait_for(lock, timeout, [this]{ return !l.empty(); }))
    return false;

  t = l.back();
  l.pop();
  return true;
}

template <class T>
bool ThreadSafeList<T>::try_pop_front(T& t, std::chrono::milliseconds timeout)
{
  std::unique_lock<std::mutex> lock(m);
  if (!c.wait_for(lock, timeout, [this]{ return !l.empty(); }))
    return false;

  t = l.front();
  l.pop();
  return true;
}

template <class T>
bool ThreadSafeList<T>::try_wait(std::chrono::milliseconds timeout)
{
  std::unique_lock<std::mutex> lock(m);
  if (!c.wait_for(lock, timeout, [this]{ return !l.empty(); }))
    return false;

  return true;
}

template <class T>
bool ThreadSafeList<T>::clone(std::list<T>& t)
{
  std::unique_lock<std::mutex> lock(m);
  t = l;
  l.clear();
  return true;
}

_END_ORPC_NAMESPACE_

#endif  //! #ifndef _ORPC_THREAD_SAFE_LIST_H_
