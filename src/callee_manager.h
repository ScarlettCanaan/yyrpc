#ifndef YY_CALLEE_MANAGER_H_
#define YY_CALLEE_MANAGER_H_

#include "async_result.h"
#include <unordered_map>
#include "callback.h"

class CalleeManager
{
public:
  static CalleeManager& GetInstance();

public:
  template <typename ... Args>
  bool BindApi(const std::string& method_name, const std::function<bool(Args...)>& func);

  const std::shared_ptr<ICallback>& GetImpl(const std::string& method_name) const;
private:
  std::unordered_map<std::string, std::shared_ptr<ICallback>> m_callees;
private:
  CalleeManager();
  ~CalleeManager();
};

template <typename ... Args>
bool CalleeManager::BindApi(const std::string& method_name, const std::function<bool(Args...)>& func)
{
  if (m_callees.find(method_name) != m_callees.end())
    return false;

  std::shared_ptr<ICallback> p(
    new TRpcCallback<bool(typename std::remove_cv<typename std::remove_reference<Args>::type>::type...)>(func));

  m_callees[method_name] = p;
  return true;
}

#endif  //! #ifndef YY_CALL_MANAGER_H_
