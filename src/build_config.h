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

#ifndef ORPC_BUILD_CONFIG_H_
#define ORPC_BUILD_CONFIG_H_

//WIN32 state-thread don't support exception
#ifdef WIN32
#define NOTHROW __declspec(nothrow)
#else
#define NOTHROW 
#endif

#ifndef WIN32
#define sprintf_s snprintf
#endif

#ifndef WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#define _START_ORPC_NAMESPACE_      namespace orpc {

#define _END_ORPC_NAMESPACE_        }

#define ORPC_USE_GLOG

#ifdef ORPC_USE_GLOG
#include "glog/logging.h"
#define ORPC_LOG LOG
#else
#include <iostream>
#define ORPC_LOG(x) std::cout
#endif  //! #ifdef ORPC_USE_GLOG

//#define ORPC_USE_FIBER

#ifdef ORPC_USE_FIBER
#include "st.h"
#endif  //! #ifdef ORPC_USE_FIBER

#define ORPC_SUPPROT_HTTP

#ifdef ORPC_SUPPROT_HTTP
#include "http_parser.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"

using namespace rapidjson;
#endif  //! #ifdef ORPC_SUPPROT_HTTP

#define USE_MSGPACK

#ifdef USE_MSGPACK
#include "msgpack.hpp"
#endif //! #ifdef USE_MSGPACK

#endif // !ORPC_BUILD_CONFIG_H_
