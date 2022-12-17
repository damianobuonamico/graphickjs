#pragma once

#include "../command.h"
#include "../../math/vec2.h"

class ChangeVec2Command: public Command {
public:
  ChangeVec2Command(vec2& old_value, vec2& new_value)
    : m_value(old_value), m_new_value(new_value), m_old_value({}) {}

  virtual void execute() override {
    m_old_value = m_value;
    m_value = m_new_value;
  }

  virtual void undo() override {
    m_value = m_old_value;
  }

  virtual bool merge_with(std::unique_ptr<Command>& command) override {
    ChangeVec2Command* casted_command = dynamic_cast<ChangeVec2Command*>(command.get());
    if (casted_command == nullptr || &casted_command->m_value != &this->m_value) return false;

    casted_command->m_new_value = this->m_new_value;

    return true;
  }

  virtual uintptr_t pointer() override {
    return reinterpret_cast<uintptr_t>(&m_value);
  }
private:
  vec2& m_value;
  vec2 m_new_value;
  vec2 m_old_value;
};
