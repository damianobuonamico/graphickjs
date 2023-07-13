#pragma once

#include "../command_history.h"
#include "../commands/primitive_commands.h"

#include <memory>

namespace Graphick::History {

  class BoolValue {
  public:
    BoolValue() : m_value(false) {};
    BoolValue(const bool value) : m_value(value) {};

    inline bool get() const { return m_value; };

    void set(const bool value) {
      if (m_value == value) return;
      CommandHistory::add(std::make_unique<ChangePrimitiveCommand<bool>>(m_value, value));
    };
  private:
    bool m_value;
  };

}
