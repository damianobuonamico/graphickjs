#pragma once

#include "../history/command_history.h"
#include "../history/commands/primitive_commands.h"

#include <memory>

class IntValue {
public:
  IntValue(): m_value(0) {};
  IntValue(const int value): m_value(value) {};

  inline int get() const { return m_value + m_delta; };
  inline int delta() const { return m_delta; };

  void set(const int value) {
    if (m_value == value) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<int>>(m_value, value));
    m_delta = 0;
  };

  void add(const int amount) {
    if (amount == 0) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<int>>(m_value, m_value + amount));
  };

  void set_delta(const int value) {
    m_delta = value;
  };

  void add_delta(const int amount) {
    m_delta += amount;
  };

  void move_to(const int value) {
    int delta = value - get();
    m_delta += delta;
  };

  void apply() {
    if (m_delta == 0) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<int>>(m_value, get()));
    m_delta = 0;
  };
private:
  int m_value;
  int m_delta = 0;
};
