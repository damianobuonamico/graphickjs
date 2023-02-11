#pragma once

#include <utility>

template<typename D>
class DataPointer {
public:
  template<typename ...Arg>
  DataPointer(Arg&&... arg): m_data{ std::forward<Arg>(arg)... } {}

  D* operator->() {
    return &m_data;
  }

  const D* operator->() const {
    return &m_data;
  }
private:
  D m_data;
};
