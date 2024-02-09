#include "common.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../history/values.h"

#include "../../../math/math.h"
#include "../../../math/matrix.h"

#include "../../../renderer/geometry/path.h"

#include "../../../utils/console.h"

namespace Graphick::Editor::Input {

  /* -- Methods -- */

  void handle_pointer_move(
    Renderer::Geometry::Path& path,
    Renderer::Geometry::ControlPoint& vertex,
    const mat2x3& transform,
    bool create_handles, bool keep_in_handle_length, bool swap_in_out,
    int* direction, Graphick::History::Vec2Value* in_use_handle
  ) {
    // mat2x3 inverse_transform = Math::inverse(transform);

    // vec2 pointer_position = inverse_transform * InputManager::pointer.scene.position;
    // vec2 pointer_origin = inverse_transform * InputManager::pointer.scene.origin;
    // vec2 pointer_last = inverse_transform * (InputManager::pointer.scene.position - InputManager::pointer.scene.movement);
    // vec2 pointer_delta = pointer_position - pointer_origin;
    // vec2 pointer_movement = pointer_position - pointer_last;

    // auto handles = path.relative_handles(vertex.id);

    // if (InputManager::keys.space) {
    //   vertex.add_delta(pointer_movement);

    //   if (handles.in_handle) handles.in_handle->add_delta(pointer_movement);
    //   if (handles.out_handle) handles.out_handle->add_delta(pointer_movement);

    //   return;
    // }

    // if (in_use_handle && in_use_handle != handles.out_handle) {
    //   std::swap(handles.in_handle, handles.out_handle);
    //   std::swap(handles.in_segment, handles.out_segment);
    // }

    // if (path.empty()) {
    //   if (create_handles && !handles.out_handle) {
    //     path.create_out_handle(pointer_origin);
    //     handles.out_handle = path.out_handle_ptr()->get();
    //   }

    //   if (handles.out_handle) handles.out_handle->set_delta(pointer_delta);

    //   if (InputManager::keys.alt) return;

    //   if (create_handles && !handles.in_handle) {
    //     path.create_in_handle(pointer_origin);
    //     handles.in_handle = path.in_handle_ptr()->get();
    //   }

    //   if (handles.in_handle) handles.in_handle->move_to(2.0f * vertex.get() - pointer_position);

    //   return;
    // }

    // if (direction) {
    //   if (*direction == 0) {
    //     float cos = 0;

    //     if (handles.out_handle) {
    //       cos = Math::dot(-pointer_delta, handles.out_handle->get() - vertex.get());
    //     } else if (handles.out_segment) {
    //       cos = Math::dot(-pointer_delta, (handles.out_segment->has_p2() ? handles.out_segment->p2() : handles.out_segment->p3()) - vertex.get());
    //     }

    //     if (cos > 0) *direction = -1;
    //     else *direction = 1;
    //   }

    //   if (*direction < 0) {
    //     std::swap(handles.in_handle, handles.out_handle);
    //     std::swap(handles.in_segment, handles.out_segment);
    //   }
    // }

    // vec2 out_handle_position = pointer_position;
    // vec2 in_handle_position = 2.0f * vertex.get() - pointer_position;

    // if (swap_in_out) {
    //   std::swap(handles.in_segment, handles.out_segment);
    //   std::swap(handles.in_handle, handles.out_handle);
    //   std::swap(out_handle_position, in_handle_position);
    // }

    // bool should_reverse_out = (!direction && path.reversed()) || (direction && *direction > 0);

    // if (create_handles && !handles.out_handle) {
    //   if (handles.out_segment) {
    //     if (should_reverse_out) {
    //       handles.out_segment->create_p1(pointer_position);
    //       handles.out_handle = handles.out_segment->p1_ptr().lock().get();
    //     } else {
    //       handles.out_segment->create_p2(pointer_position);
    //       handles.out_handle = handles.out_segment->p2_ptr().lock().get();
    //     }
    //   } else {
    //     if (should_reverse_out) {
    //       path.create_in_handle(pointer_origin);
    //       handles.out_handle = path.in_handle_ptr()->get();
    //     } else {
    //       path.create_out_handle(pointer_origin);
    //       handles.out_handle = path.out_handle_ptr()->get();
    //     }
    //   }
    // }

    // if (handles.out_handle) handles.out_handle->move_to(out_handle_position);

    // if (
    //   InputManager::keys.alt ||
    //   Math::is_almost_equal(handles.out_handle->get(), vertex.get()) ||
    //   (!handles.in_handle && keep_in_handle_length) ||
    //   (!create_handles && !handles.in_handle)
    //   ) return;

    // if (!handles.in_handle) {
    //   if ((!direction && path.reversed() == swap_in_out) || (direction && *direction > 0)) {
    //     handles.in_segment->create_p2(pointer_position);
    //     handles.in_handle = handles.in_segment->p2_ptr().lock().get();
    //   } else {
    //     handles.in_segment->create_p1(pointer_position);
    //     handles.in_handle = handles.in_segment->p1_ptr().lock().get();
    //   }
    // }

    // if (keep_in_handle_length) {
    //   vec2 dir = Math::normalize(vertex.get() - handles.out_handle->get());
    //   float length = Math::length(handles.in_handle->get() - handles.in_handle->delta() - vertex.get() + vertex.delta());

    //   in_handle_position = dir * length + vertex.get();
    // }

    // handles.in_handle->move_to(in_handle_position);
  }

  /* -- SelectionRect -- */

  SelectionRect::SelectionRect(bool dashed) :
    m_position({ 0.0f, 0.0f }), m_anchor_position({ 0.0f, 0.0f }),
    m_dashed(dashed)
  {
    m_path.move_to({ -0.5f, -0.5f });
    m_path.line_to({ 0.5f, -0.5f });
    m_path.line_to({ 0.5f, 0.5f });
    m_path.line_to({ -0.5f, 0.5f });
    m_path.close();
  }

  rect SelectionRect::bounding_rect() const {
    return transform() * m_path.bounding_rect();
  }

  mat2x3 SelectionRect::transform() const {
    return Math::translate(Math::rotate(Math::scale(mat2x3{ 1.0f }, m_size), m_angle), m_position + m_size / 2.0f);
  }

  void SelectionRect::set(const vec2 position) {
    m_anchor_position = m_position = position;
    m_active = true;

    size({ 0.0f, 0.0f });
  }

  void SelectionRect::reset() {
    m_position = m_anchor_position;
    m_active = false;

    size({ 0.0f, 0.0f });
  }

  /* -- Manipulator -- */

  bool Manipulator::update() {
    Selection& selection = Editor::scene().selection;

    if (selection.empty()) return m_active = false;

    update_positions(selection.bounding_rect());

    return true;
  }

  bool Manipulator::on_pointer_down(const float threshold) {
    m_start_transform = transform();
    m_threshold = threshold;
    m_cache.clear();

    if (!m_active) {
      m_active_handle = HandleNone;
      m_in_use = false;

      return false;
    }

    vec2 transformed_position = inverse(transform()) * InputManager::pointer.scene.position;
    vec2 handle_size = vec2{ threshold } / m_size;

    for (int i = 0; i < HandleNone; i++) {
      vec2 handle_position = m_handles[i];

      if (Math::is_point_in_ellipse(transformed_position, handle_position, i >= 8 ? handle_size * 2.0f : handle_size)) {

        switch (i) {
        case N:
          m_center = m_handles[S];
          break;
        case S:
          m_center = m_handles[N];
          break;
        case E:
          m_center = m_handles[W];
          break;
        case W:
          m_center = m_handles[E];
          break;
        case NE:
          m_center = m_handles[SW];
          break;
        case NW:
          m_center = m_handles[SE];
          break;
        case SE:
          m_center = m_handles[NW];
          break;
        case SW:
          m_center = m_handles[NE];
          break;
        default:
          if (Math::is_point_in_rect(transformed_position, { vec2{ -0.5f }, vec2{ 0.5f } })) return false;
        }

        m_start_bounding_rect = bounding_rect();
        m_active_handle = static_cast<HandleType>(i);
        m_handle = handle_position;
        m_in_use = true;

        if (i > SW) {
          m_center = vec2{ 0.0f };
        }

        Scene& scene = Editor::scene();
        auto& selected = scene.selection.selected();

        m_cache.reserve(selected.size());

        for (const auto& [id, _] : selected) {
          if (scene.has_entity(id)) {
            Entity entity = scene.get_entity(id);

            if (entity.has_component<TransformComponent>()) {
              //m_cache.push_back(entity.get_component<TransformComponent>()._value());
            }
          }
        }

        return true;
      }
    }

    m_active_handle = HandleNone;
    m_in_use = false;

    return false;
  }

  void Manipulator::on_pointer_move() {
    if (!m_active) return;

    if (m_active_handle > SW) on_rotate_pointer_move();
    else on_scale_pointer_move();
  }

  void Manipulator::on_pointer_up() {
    m_active_handle = HandleNone;
    m_in_use = false;

    for (Graphick::History::Mat2x3Value* matrix : m_cache) {
      matrix->apply();
    }

    m_cache.clear();

    update();
  }

  bool Manipulator::on_key(const bool down, const KeyboardKey key) {
    if (!m_active) return false;

    if (InputManager::keys.shift_state_changed || InputManager::keys.alt_state_changed) {
      on_pointer_move();
    }

    return true;
  }

  void Manipulator::update_positions(const rrect& bounding_rect) {
    vec2 rect_size = bounding_rect.size();

    set(bounding_rect.min);
    size(rect_size);
    angle(bounding_rect.angle);

    if (std::abs(rect_size.x) >= m_threshold * 7.0f) {
      m_handles[N] = m_handles[RN] = vec2{ 0.0f, -0.5f };
      m_handles[S] = m_handles[RS] = vec2{ 0.0f, 0.5f };
    } else {
      m_handles[N] = m_handles[RN] = m_handles[S] = m_handles[RS] = std::numeric_limits<vec2>::max();
    }

    if (std::abs(rect_size.y) >= m_threshold * 7.0f) {
      m_handles[E] = m_handles[RE] = vec2{ 0.5f, 0.0f };
      m_handles[W] = m_handles[RW] = vec2{ -0.5f, 0.0f };
    } else {
      m_handles[E] = m_handles[RE] = m_handles[W] = m_handles[RW] = std::numeric_limits<vec2>::max();
    }

    m_handles[NW] = m_handles[RNW] = vec2{ -0.5f, -0.5f };
    m_handles[NE] = m_handles[RNE] = vec2{ 0.5f, -0.5f };
    m_handles[SE] = m_handles[RSE] = vec2{ 0.5f, 0.5f };
    m_handles[SW] = m_handles[RSW] = vec2{ -0.5f, 0.5f };
  }

  void Manipulator::on_scale_pointer_move() {
    vec2 local_center = InputManager::keys.alt ? vec2{ 0.0f } : m_center;

    vec2 old_delta = m_handle - local_center;
    vec2 delta = inverse(m_start_transform) * InputManager::pointer.scene.position - local_center;

    vec2 magnitude = delta / old_delta;
    uint8_t axial = 0;    /* 0 = none, 1 = x, 2 = y */

    if (m_active_handle == N || m_active_handle == S) {
      magnitude.x = 1.0f;
      axial = 1;
    } else if (m_active_handle == E || m_active_handle == W) {
      magnitude.y = 1.0f;
      axial = 2;
    }

    if (InputManager::keys.shift) {
      if (axial == 1) {
        magnitude.x = magnitude.y;
      } else if (axial == 2) {
        magnitude.y = magnitude.x;
      } else {
        if (magnitude.x > magnitude.y) {
          magnitude.x = magnitude.y;
        } else {
          magnitude.y = magnitude.x;
        }
      }
    }

    vec2 center = m_start_transform * local_center;

    rrect new_bounding_rect = {
      Math::scale(m_start_bounding_rect.min, center, magnitude),
      Math::scale(m_start_bounding_rect.max, center, magnitude)
    };

    update_positions(new_bounding_rect);

    for (Graphick::History::Mat2x3Value* matrix : m_cache) {
      matrix->set_delta(mat2x3{ 0.0f });
      matrix->scale(center, magnitude);
    }
  }

  void Manipulator::on_rotate_pointer_move() {
    float angle = Math::angle(m_handle - m_center, inverse(m_start_transform) * InputManager::pointer.scene.position - m_center);
    float sin_angle = std::sinf(angle);
    float cos_angle = std::cosf(angle);

    vec2 center = m_start_transform * m_center;

    rrect new_bounding_rect = {
      m_start_bounding_rect,
      angle
    };

    update_positions(new_bounding_rect);

    for (Graphick::History::Mat2x3Value* matrix : m_cache) {
      matrix->set_delta(mat2x3{ 0.0f });
      matrix->rotate(center, sin_angle, cos_angle);
    }
  }

}
