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

#ifndef TYPE_TRAITS_IS_CONTAINER_H_
#define TYPE_TRAITS_IS_CONTAINER_H_

#include <typeinfo>
#include <map>
#include <string>
#include <list>
#include <vector>
#include <set>
#include <unordered_map>

_START_ORPC_NAMESPACE_

// Basic template; specialize to derive from std::true_type for all desired container types
template<typename T> 
struct is_seq_container : public std::false_type { };

template<typename T> 
struct is_map_container : public std::false_type { };

// Mark vector as a seq_container
template<typename T, typename TAllocator> 
struct is_seq_container<std::vector<T, TAllocator> > : public std::true_type{};

// Mark list as a seq_container
template<typename T, typename TAllocator> 
struct is_seq_container<std::list<T, TAllocator> > : public std::true_type{};

// Mark set as a seq_container
template<typename T, typename TTraits, typename TAllocator> 
struct is_seq_container<std::set<T, TTraits, TAllocator> > : public std::true_type{};

// Mark map as a map_container
template<typename TKey, typename TValue, typename TTraits, typename TAllocator> 
struct is_map_container<std::map<TKey, TValue, TTraits, TAllocator> > : public std::true_type{};

// Mark map as a map_container
template<typename TKey, typename TValue, typename TTraits, typename TAllocator> 
struct is_map_container<std::unordered_map<TKey, TValue, TTraits, TAllocator> > : public std::true_type{};

_END_ORPC_NAMESPACE_

#endif  //! #ifndef TYPE_TRAITS_IS_CONTAINER_H_
