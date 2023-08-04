#pragma once

#include "../../history/values.h"

namespace Graphick::Renderer::Geometry {

  class ControlPoint {
  public:
    ControlPoint() : m_value() {}
    ControlPoint(const vec2 value) : m_value(value) {}
    ControlPoint(float x, float y) : m_value({ x, y }) {}
    ~ControlPoint() = default;

    inline vec2 get() const { return m_value.get(); }
    inline vec2 delta() const { return m_value.delta(); }

    void set_relative_handle(std::weak_ptr<History::Vec2Value> handle) {
      m_relative_handles.push_back(handle);

      m_relative_handles.erase(std::remove_if(
        m_relative_handles.begin(), m_relative_handles.end(),
        [](auto& handle) { return handle.expired(); }),
        m_relative_handles.end()
      );
    }

    void remove_relative_handle(std::weak_ptr<History::Vec2Value> handle) {
      auto handle_ptr = handle.lock();
      if (!handle_ptr) return;

      int index = -1;

      for (int i = 0; i < m_relative_handles.size(); i++) {
        auto h = m_relative_handles[i];
        if (auto h_ptr = h.lock()) {
          if (h_ptr.get() == handle_ptr.get()) {
            index = i;
            break;
          }
        }
      }

      if (index == -1) return;

      m_relative_handles.erase(m_relative_handles.begin() + index);
    }

    void reset_relative_handles() {
      m_relative_handles.clear();
    }

    void set(const vec2 value) {
      vec2 delta = value - m_value.get();
      if (Math::is_zero(delta)) return;

      m_value.set(value);

      for (auto& handle : m_relative_handles) {
        if (auto handle_ptr = handle.lock()) {
          handle_ptr->add(delta);
        }
      }
    }

    void add(const vec2 amount) {
      if (Math::is_zero(amount)) return;

      m_value.add(amount);

      for (auto& handle : m_relative_handles) {
        if (auto handle_ptr = handle.lock()) {
          handle_ptr->add(amount);
        }
      }
    }

    void set_delta(const vec2 value) {
      vec2 delta = value - m_value.delta();
      if (Math::is_zero(delta)) return;

      m_value.set_delta(value);

      for (auto& handle : m_relative_handles) {
        if (auto handle_ptr = handle.lock()) {
          handle_ptr->add_delta(delta);
        }
      }
    }

    void add_delta(const vec2 amount) {
      if (Math::is_zero(amount)) return;

      m_value.add_delta(amount);

      for (auto& handle : m_relative_handles) {
        if (auto handle_ptr = handle.lock()) {
          handle_ptr->add_delta(amount);
        }
      }
    }

    void move_to(const vec2 value) {
      vec2 delta = value - m_value.get();
      if (Math::is_zero(delta)) return;

      m_value.move_to(value);

      for (auto& handle : m_relative_handles) {
        if (auto handle_ptr = handle.lock()) {
          handle_ptr->add_delta(delta);
        }
      }
    }

    void apply() {
      m_value.apply();

      for (auto& handle : m_relative_handles) {
        if (auto handle_ptr = handle.lock()) {
          handle_ptr->apply();
        }
      }
    }
  private:
    History::Vec2Value m_value;
    std::vector<std::weak_ptr<History::Vec2Value>> m_relative_handles;
  };

}
