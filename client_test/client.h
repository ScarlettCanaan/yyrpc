#ifndef YYRPC_CLIENT_H_
#define YYRPC_CLIENT_H_

#include "build_config.h"

int client_caller();

int bench_single_thread_call();

int NOTHROW bench_tiny_call();
#endif //! #ifndef YYRPC_CLIENT_H_
