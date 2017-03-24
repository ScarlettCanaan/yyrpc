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

#ifndef __BOOST_INDEX_SEQUENCE_H_
#define __BOOST_INDEX_SEQUENCE_H_

_START_ORPC_NAMESPACE_
//this implement copy from boost

namespace __boost {

template<std::size_t ... I>
struct index_sequence {
  using type = index_sequence;
  using value_type = std::size_t;
  static std::size_t size() {
    return sizeof ... (I);
  }
};

template<typename Seq1, typename Seq2>
struct concat_sequence;

template<std::size_t ... I1, std::size_t ... I2 >
struct concat_sequence<index_sequence<I1 ...>, index_sequence<I2 ...>> : public index_sequence<I1 ..., (sizeof ... (I1) + I2) ...> {
};

template<std::size_t I>
struct make_index_sequence : public concat_sequence< typename make_index_sequence<I / 2>::type,
                                                     typename make_index_sequence<I - I / 2>::type > {
};

template<>
struct make_index_sequence<0> : public index_sequence<> {
};

template<>
struct make_index_sequence<1> : public index_sequence<0> {
};

template<typename ... T>
using index_sequence_for = make_index_sequence<sizeof ... (T)>;

} // ! namespace __boost

_END_ORPC_NAMESPACE_

#endif  //! #ifndef __BOOST_INDEX_SEQUENCE_H_

