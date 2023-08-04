#include "values.h"

#include "command_history.h"
#include "commands.h"

namespace Graphick::History {

  /* -- BoolValue -- */

  void BoolValue::set(const bool value) {
    if (m_value == value) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<bool>>(m_value, value));
  };

  /* -- FloatValue -- */

  void FloatValue::set(const float value) {
    if (m_value == value) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<float>>(m_value, value));
    m_delta = 0.0f;
  };

  void FloatValue::add(const float amount) {
    if (amount == 0.0f) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<float>>(m_value, m_value + amount));
  };

  void FloatValue::apply() {
    if (m_delta == 0.0f) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<float>>(m_value, get()));
    m_delta = 0.0f;
  };

  /* -- IntValue -- */

  void IntValue::set(const int value) {
    if (m_value == value) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<int>>(m_value, value));
    m_delta = 0;
  };

  void IntValue::add(const int amount) {
    if (amount == 0) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<int>>(m_value, m_value + amount));
  };

  void IntValue::apply() {
    if (m_delta == 0) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<int>>(m_value, get()));
    m_delta = 0;
  };

  /* -- Vec2Value -- */

  void Vec2Value::set(const vec2& value) {
    if (m_value == value) return;
    CommandHistory::add(std::make_unique<ChangeVec2Command>(m_value, value));
    Math::zero(m_delta);
  };

  void Vec2Value::add(const vec2& amount) {
    if (Math::is_zero(amount)) return;
    CommandHistory::add(std::make_unique<ChangeVec2Command>(m_value, m_value + amount));
  };

  void Vec2Value::apply() {
    if (Math::is_zero(m_delta)) return;
    CommandHistory::add(std::make_unique<ChangeVec2Command>(m_value, get()));
    Math::zero(m_delta);
  };

}
