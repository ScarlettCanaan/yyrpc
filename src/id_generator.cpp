#include "id_generator.h"

#ifdef WIN32
__declspec(thread) unsigned int session_id = 1;
#else
thread_local unsigned int session_id = 1;
#endif

uint32_t get_sessionid()
{
  return ++session_id;
}
