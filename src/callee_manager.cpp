#include "callee_manager.h"
#include "util/util.h"
#include <iostream>

CalleeManager& CalleeManager::GetInstance()
{
  static CalleeManager inst;
  return inst;
}

CalleeManager::CalleeManager()
{
}

CalleeManager::~CalleeManager()
{
}

const std::shared_ptr<ICallback>& CalleeManager::GetImpl(const std::string& method_name) const
{
  static std::shared_ptr<ICallback> null;
  auto it = m_callees.find(method_name);
  if (it == m_callees.end())
    return null;

  return it->second;
}

