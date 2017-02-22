#ifndef YY_CALL_MANAGER_H_
#define YY_CALL_MANAGER_H_

#include "async_result.h"
#include <unordered_map>

class EndPoint;

struct CallPacket
{
  CallPacket(const std::shared_ptr<EndPoint>& endpoint, uint32_t session_id, std::stringstream& s);

  uint32_t m_sessionId;
  int64_t  m_callTime;
  std::string m_data;
  std::shared_ptr<EndPoint> m_endpoint;
};

class CallerManager
{
public:
  static CallerManager& GetInstance();
public:
  template<typename R>
  std::shared_ptr<AsyncResult<R>> Call(const std::shared_ptr<EndPoint>& endpoint, uint32_t session_id, std::stringstream& s);

  template<typename R>
  void OnResult(uint32_t session_id, const R& r);
private:
  void QueuePacket(const std::shared_ptr<CallPacket>& pPacket);
private:
  std::unordered_map<int32_t, std::shared_ptr<IAsyncResult>> m_pendingResult;
private:
  CallerManager();
  ~CallerManager();
};

template<typename R>
std::shared_ptr<AsyncResult<R>> CallerManager::Call(const std::shared_ptr<EndPoint>& endpoint, uint32_t session_id, std::stringstream& s)
{
  std::shared_ptr<CallPacket> pPacket = std::make_shared<CallPacket>(endpoint, session_id, s);
  QueuePacket(pPacket);

  std::shared_ptr<AsyncResult<R>> pResult = std::make_shared<AsyncResult<R>>(session_id);
  m_pendingResult[session_id] = pResult;
  return pResult;
}

template<typename R>
void CallerManager::OnResult(uint32_t session_id, const R& r)
{
  auto it = m_pendingResult.find(session_id);
  if (it == m_pendingResult.end())
    return;
  it->second->set_result(&r);
}

#endif  //! #ifndef YY_CALL_MANAGER_H_
