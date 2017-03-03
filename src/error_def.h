#ifndef YYRPC_ERROR_DEF_H_
#define YYRPC_ERROR_DEF_H_

#define YYRPC_ERROR_UNKNOWN            -1
#define YYRPC_ERROR_SUCESS              0
#define YYRPC_ERROR_CALL_TIMEOUT        1
#define YYRPC_ERROR_CONNECT_FINAL_FAIL  2
#define YYRPC_ERROR_DISCONNECT          3
#define YYRPC_ERROR_SERI_FAILED         4
#define YYRPC_ERROR_CANT_FIND_METHOD    5
#define YYRPC_ERROR_ENDPOINT_INVALID    6

inline const char* error_id_to_string(int32_t id)
{
  return "";
}

#endif // !YYRPC_ERROR_DEF_H_
