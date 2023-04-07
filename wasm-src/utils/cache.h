#pragma once

#include "uuid.h"

template <typename T>
struct Cached {
  Cached() {}

  inline const T& get() const { return m_value; }
  inline const UUID& id() const { return m_id; }

  inline void set(T value, UUID id) { m_value = value; m_id = id; }
private:
  T m_value;
  UUID m_id;
};
