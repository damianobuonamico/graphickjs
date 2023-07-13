#pragma once

#include "../../math/vector.h"
// TODO: move in cpp file
#include "../command_history.h"
#include "../commands/vec_commands.h"

#include <memory>

namespace Graphick::History {

  class Vec2Value {
  public:
    Vec2Value() : m_value(0.0f) {};
    Vec2Value(const vec2 value) : m_value(value) {};
    Vec2Value(float x, float y) : m_value({ x, y }) {};
    ~Vec2Value() = default;

    inline vec2 get() const { return m_value + m_delta; };
    inline vec2 delta() const { return m_delta; };

    void set(const vec2& value) {
      if (m_value == value) return;
      CommandHistory::add(std::make_unique<ChangeVec2Command>(m_value, value));
      Math::zero(m_delta);
    };

    void add(const vec2& amount) {
      if (Math::is_zero(amount)) return;
      CommandHistory::add(std::make_unique<ChangeVec2Command>(m_value, m_value + amount));
    };

    void set_delta(const vec2& value) {
      m_delta = value;
    };

    void add_delta(const vec2& amount) {
      m_delta += amount;
    };

    void move_to(const vec2& value) {
      vec2 delta = value - get();
      m_delta += delta;
    };

    void apply() {
      if (Math::is_zero(m_delta)) return;
      CommandHistory::add(std::make_unique<ChangeVec2Command>(m_value, get()));
      Math::zero(m_delta);
    };
  private:
    vec2 m_value;
    vec2 m_delta{ 0.0f };
  };

}
