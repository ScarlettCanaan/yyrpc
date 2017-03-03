#ifndef YYRPC_BUILD_CONFIG_H_
#define YYRPC_BUILD_CONFIG_H_

#ifdef WIN32
#define NOTHROW __declspec(nothrow)
#else
#define NOTHROW 
#endif

#define HTTP_METHOD     1
#define BINARY_METHOD   1

#endif // !YYRPC_BUILD_CONFIG_H_
