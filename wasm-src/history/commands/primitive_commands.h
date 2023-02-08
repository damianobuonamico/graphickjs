#pragma once

#include "../command.h"

class ChangeBoolCommand: public Command {
public:
  ChangeBoolCommand(bool& old_value, const bool new_value)
    : m_value(old_value), m_new_value(new_value), m_old_value(old_value) {}

  virtual void execute() override {
    m_old_value = m_value;
    m_value = m_new_value;
  }

  virtual void undo() override {
    m_value = m_old_value;
  }

  virtual bool merge_with(std::unique_ptr<Command>& command) override {
    ChangeBoolCommand* casted_command = dynamic_cast<ChangeBoolCommand*>(command.get());
    if (casted_command == nullptr || &casted_command->m_value != &this->m_value) return false;

    casted_command->m_new_value = this->m_new_value;

    return true;
  }

  virtual uintptr_t pointer() override {
    return reinterpret_cast<uintptr_t>(&m_value);
  }
private:
  bool& m_value;
  bool m_new_value;
  bool m_old_value;
};
