#ifndef YY_CALL_MANAGER_H_
#define YY_CALL_MANAGER_H_

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <mutex>

class IAsyncResult;

class CallerManager
{
public:
  static CallerManager& GetInstance();
private:
  CallerManager();
  ~CallerManager();
public:
  bool OnResult(std::thread::id id, std::shared_ptr<IAsyncResult> result);

  bool PumpMessage();
private:
  mutable std::mutex m_resultMutex;
  std::map<std::thread::id, std::set<std::shared_ptr<IAsyncResult>>> m_specThreadResult;
};

#endif  //! #ifndef YY_CALL_MANAGER_H_
