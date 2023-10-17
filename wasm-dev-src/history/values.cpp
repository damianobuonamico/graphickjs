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

  /* -- UUIDValue -- */

  void UUIDValue::set(const uuid value) {
    if (m_value == value) return;
    CommandHistory::add(std::make_unique<ChangePrimitiveCommand<uuid>>(m_value, value));
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

  /* -- VectorValue -- */

  template<typename T>
  void VectorValue<T>::push_back(const T& value) {
    CommandHistory::add(std::make_unique<InsertInVectorCommand<T>>(&m_value, value));
  }

  template<typename T>
  void VectorValue<T>::insert(const T& value, int index) {
    if (m_value.size() < index || index < 0) return;
    History::CommandHistory::add(std::make_unique<History::InsertInVectorCommand<T>>(&m_value, value, index));
  }

  template<typename T>
  void VectorValue<T>::pop_back() {
    erase((int)m_value.size() - 1);
  }

  template<typename T>
  void VectorValue<T>::erase(const T& value) {
    History::CommandHistory::add(std::make_unique<History::EraseFromVectorCommand<T>>(&m_value, value));
  }

  template<typename T>
  void VectorValue<T>::erase(int index) {
    if (m_value.size() <= index || index < 0) return;
    History::CommandHistory::add(std::make_unique<History::EraseFromVectorCommand<T>>(&m_value, m_value.at(index), index));
  }

  template<typename T>
  void VectorValue<T>::clear() {
    if (m_value.empty()) return;

    for (int i = (int)m_value.size() - 1; i >= 0; i--) {
      History::CommandHistory::add(std::make_unique<History::EraseFromVectorCommand<T>>(&m_value, m_value.at(i), i));
    }
  }

}

namespace entt {
  enum class entity;
}

namespace Graphick::Renderer::Geometry {
  class Segment;
}

namespace Graphick::History {
  // Register here types that are used to instantiate the template to avoid linking errors
  template class VectorValue<entt::entity>;
  template class VectorValue<std::shared_ptr<Renderer::Geometry::Segment>>;
}


