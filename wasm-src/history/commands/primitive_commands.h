#pragma once

#include "../command.h"

template <typename T>
class ChangePrimitiveCommand: public Command {
public:
  ChangePrimitiveCommand(T& old_value, const T new_value)
    : m_value(old_value), m_new_value(new_value), m_old_value(old_value) {}

  virtual void execute() override {
    m_old_value = m_value;
    m_value = m_new_value;
  }

  virtual void undo() override {
    m_value = m_old_value;
  }

  virtual bool merge_with(std::unique_ptr<Command>& command) override {
    ChangePrimitiveCommand<T>* casted_command = dynamic_cast<ChangePrimitiveCommand<T>*>(command.get());
    if (casted_command == nullptr || &casted_command->m_value != &this->m_value) return false;

    casted_command->m_new_value = this->m_new_value;

    return true;
  }

  virtual uintptr_t pointer() override {
    return reinterpret_cast<uintptr_t>(&m_value);
  }
private:
  T& m_value;
  T m_new_value;
  T m_old_value;
};
