#pragma once

#include "../history/command_history.h"
#include "../history/commands/primitive_commands.h"

#include <memory>

class FloatValue {
public:
  FloatValue(): m_value(0.0f) {};
  FloatValue(const float value): m_value(value) {};

  inline float get() const { return m_value + m_delta; };
  inline float delta() const { return m_delta; };

  void set(const float value) {
    if (m_value == value) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<float>>(m_value, value));
    m_delta = 0.0f;
  };

  void add(const float amount) {
    if (amount == 0.0f) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<float>>(m_value, m_value + amount));
  };

  void set_delta(const float value) {
    m_delta = value;
  };

  void add_delta(const float amount) {
    m_delta += amount;
  };

  void move_to(const float value) {
    float delta = value - get();
    m_delta += delta;
  };

  void apply() {
    if (m_delta == 0.0f) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<float>>(m_value, get()));
    m_delta = 0.0f;
  };
private:
  float m_value;
  float m_delta = 0.0f;
};
