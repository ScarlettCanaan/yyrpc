#ifndef YYRPC_ERROR_DEF_H_
#define YYRPC_ERROR_DEF_H_

#define YYRPC_ERROR_CALL_TIMEOUT        1
#define YYRPC_ERROR_CONNECT_FINAL_FAIL  2
#define YYRPC_ERROR_DISCONNECT          3

inline const char* error_id_to_string(int32_t id)
{
  return "";
}

#endif // !YYRPC_ERROR_DEF_H_
