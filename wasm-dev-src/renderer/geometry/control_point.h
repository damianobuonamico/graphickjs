#pragma once

#include "../../history/values.h"

#include "../../utils/uuid.h"

#include <vector>

namespace Graphick::Renderer::Geometry {

  class ControlPoint {
  public:
    uuid id;
  public:
    ControlPoint() : m_value() {}
    ControlPoint(const vec2 value) : m_value(value) {}
    ControlPoint(float x, float y) : m_value(x, y) {}
    ~ControlPoint() = default;

    inline vec2 get() const { return m_value.get(); }
    inline vec2 delta() const { return m_value.delta(); }

    void set_relative_handle(std::weak_ptr<History::Vec2Value> handle);
    void remove_relative_handle(std::weak_ptr<History::Vec2Value> handle);
    void reset_relative_handles();

    void set(const vec2 value);
    void add(const vec2 amount);

    void set_delta(const vec2 value);
    void add_delta(const vec2 amount);
    void move_to(const vec2 value);

    void apply();
  private:
    History::Vec2Value m_value;
    std::vector<std::weak_ptr<History::Vec2Value>> m_relative_handles;
  };

}
