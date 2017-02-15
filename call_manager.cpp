#include "call_manager.h"
#include "util/util.h"
#include <iostream>

CallPacket::CallPacket(uint32_t session_id, std::stringstream& s)
{
  m_sessionId = session_id;
  m_data = s.str();
  GetTimeSecond(&m_callTime);
}

AsyncCallManager& AsyncCallManager::GetInstance()
{
  static AsyncCallManager inst;
  return inst;
}

AsyncCallManager::AsyncCallManager()
{
}

AsyncCallManager::~AsyncCallManager()
{
}

