#include "id_generator.h"

#ifdef WIN32
__declspec(thread) uint64_t session_id = 1;
#else
thread_local uint64_t session_id = 1;
#endif

uint64_t get_sessionid()
{
  return ++session_id;
}
