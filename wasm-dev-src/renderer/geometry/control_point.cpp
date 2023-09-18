#include "control_point.h"

namespace Graphick::Renderer::Geometry {

  void ControlPoint::set_relative_handle(std::weak_ptr<History::Vec2Value> handle) {
    History::Vec2Value* handle_ptr = handle.lock().get();

    m_relative_handles.erase(std::remove_if(
      m_relative_handles.begin(), m_relative_handles.end(),
      [handle_ptr](auto& h) { return (h.expired() || h.lock().get() == handle_ptr); }),
      m_relative_handles.end()
    );

    m_relative_handles.push_back(handle);
  }

  void ControlPoint::remove_relative_handle(std::weak_ptr<History::Vec2Value> handle) {
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

  void ControlPoint::reset_relative_handles() {
    m_relative_handles.clear();
  }

  void ControlPoint::set(const vec2 value) {
    vec2 delta = value - m_value.get();
    if (Math::is_zero(delta)) return;

    m_value.set(value);

    for (auto& handle : m_relative_handles) {
      if (auto handle_ptr = handle.lock()) {
        handle_ptr->add(delta);
      }
    }
  }

  void ControlPoint::add(const vec2 amount) {
    if (Math::is_zero(amount)) return;

    m_value.add(amount);

    for (auto& handle : m_relative_handles) {
      if (auto handle_ptr = handle.lock()) {
        handle_ptr->add(amount);
      }
    }
  }

  void ControlPoint::set_delta(const vec2 value) {
    vec2 delta = value - m_value.delta();
    if (Math::is_zero(delta)) return;

    m_value.set_delta(value);

    for (auto& handle : m_relative_handles) {
      if (auto handle_ptr = handle.lock()) {
        handle_ptr->add_delta(delta);
      }
    }
  }

  void ControlPoint::add_delta(const vec2 amount) {
    if (Math::is_zero(amount)) return;

    m_value.add_delta(amount);

    for (auto& handle : m_relative_handles) {
      if (auto handle_ptr = handle.lock()) {
        handle_ptr->add_delta(amount);
      }
    }
  }

  void ControlPoint::move_to(const vec2 value) {
    vec2 delta = value - m_value.get();
    if (Math::is_zero(delta)) return;

    m_value.move_to(value);

    for (auto& handle : m_relative_handles) {
      if (auto handle_ptr = handle.lock()) {
        handle_ptr->add_delta(delta);
      }
    }
  }

  void ControlPoint::apply() {
    m_value.apply();
  }

  void ControlPoint::deep_apply() {
    m_value.apply();

    for (auto& handle : m_relative_handles) {
      if (auto handle_ptr = handle.lock()) {
        handle_ptr->apply();
      }
    }
  }

}
