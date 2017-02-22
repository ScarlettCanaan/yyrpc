#ifndef YY_CALL_MANAGER_H_
#define YY_CALL_MANAGER_H_

#include "async_result.h"
#include <hash_map>
//#include "ds/lock_free_list.h"

struct CallPacket
{
  CallPacket(uint32_t session_id, std::stringstream& s);

  std::string m_data;
  uint32_t m_sessionId;
  int64_t  m_callTime;
};

class AsyncCallManager
{
public:
  static AsyncCallManager& GetInstance();
public:
  template<typename R>
  std::shared_ptr<AsyncResult<R>> Call(uint32_t session_id, std::stringstream& s);

  template<typename R>
  void OnResult(uint32_t session_id, const R& r);
private:
  std::hash_map<int32_t, std::shared_ptr<IAsyncResult>> m_pendingResult;
  ThreadSafeQueue<std::shared_ptr<CallPacket>> m_taskQuque;
private:
  AsyncCallManager();
  ~AsyncCallManager();
};

template<typename R>
std::shared_ptr<AsyncResult<R>> AsyncCallManager::Call(uint32_t session_id, std::stringstream& s)
{
  std::shared_ptr<CallPacket> pPacket = std::make_shared<CallPacket>(session_id, s);
  m_taskQuque.push(pPacket);

  std::shared_ptr<AsyncResult<R>> pResult = std::make_shared<AsyncResult<R>>(session_id);
  m_pendingResult[session_id] = pResult;
  return pResult;
}

template<typename R>
void AsyncCallManager::OnResult(uint32_t session_id, const R& r)
{
  auto it = m_pendingResult.find(session_id);
  if (it == m_pendingResult.end())
    return;
  it->second->SetResult(&r);
}

#endif  //! #ifndef YY_CALL_MANAGER_H_
