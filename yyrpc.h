#ifndef YYRPC_H_
#define YYRPC_H_
#include "typecheck.h"

#define SYNC_CALL(method_name, ...) \
    TYPE_CHECK(method_name, __VA_ARGS__)

#endif //! #ifndef YYRPC_H_
