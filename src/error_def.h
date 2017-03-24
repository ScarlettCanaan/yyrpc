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

#ifndef ORPC_ERROR_DEF_H_
#define ORPC_ERROR_DEF_H_

#include "build_config.h"
#include "stdint.h"

_START_ORPC_NAMESPACE_

#define ORPC_ERROR_UNKNOWN            -1
#define ORPC_ERROR_SUCESS              0
#define ORPC_ERROR_CALL_TIMEOUT        1
#define ORPC_ERROR_CONNECT_FINAL_FAIL  2
#define ORPC_ERROR_DISCONNECT          3
#define ORPC_ERROR_HEADER_FAILED       4
#define ORPC_ERROR_CANT_FIND_METHOD    5
#define ORPC_ERROR_ENDPOINT_INVALID    6
#define ORPC_ERROR_BODY_FAILED         7
#define ORPC_ERROR_AUTH_FAILED         8
#define ORPC_ERROR_MARSHAL_FAILED      9

const char* ErrorIdToString(int32_t id);

_END_ORPC_NAMESPACE_

#endif // !ORPC_ERROR_DEF_H_
