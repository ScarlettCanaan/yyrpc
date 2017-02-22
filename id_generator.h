#ifndef ID_GENERATOR_H_
#define ID_GENERATOR_H_

#ifdef WIN32
__declspec(thread) unsigned int session_id = 1;
#else
thread_local unsigned int session_id = 1;
#endif

inline uint32_t get_sessionid()
{
  return ++session_id;
}

#endif  //! #ifndef ID_GENERATOR_H_