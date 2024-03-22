/**
 * @file common.cpp
 * @brief Contains the implementation of common classes and functions used by input tools in the Graphick editor.
 */

#include "common.h"

#include "../input_manager.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../math/math.h"
#include "../../../math/matrix.h"

#include "../../../renderer/geometry/path.h"

#include "../../../utils/console.h"

namespace Graphick::Editor::Input {

  /* -- Methods -- */

  size_t translate_control_point(
    PathComponent& path,
    const size_t point_index,
    const mat2x3& transform, const vec2* override_movement,
    bool create_handles, bool keep_in_handle_length, bool translate_in_first,
    int* direction
  ) {
    const mat2x3 inverse_transform = override_movement ? mat2x3{} : Math::inverse(transform);
    const vec2 position = inverse_transform * InputManager::pointer.scene.position;
    const vec2 origin = inverse_transform * InputManager::pointer.scene.origin;

    Renderer::Geometry::Path::VertexNode node = path.data().node_at(point_index);

    if (!create_handles && point_index == node.vertex) {
      const vec2 vertex_position = path.data().point_at(node.vertex);
      const vec2 movement = override_movement ? *override_movement : position - vertex_position;

      path.translate(node.vertex, movement);

      if (node.in >= 0) path.translate(static_cast<size_t>(node.in), movement);
      if (node.out >= 0) path.translate(static_cast<size_t>(node.out), movement);
      if (node.close_vertex >= 0) path.translate(static_cast<size_t>(node.close_vertex), movement);

      return point_index;
    } else if (InputManager::keys.space) {
      const vec2 out_position = path.data().point_at(node.out);
      const vec2 movement = override_movement ? *override_movement : position - out_position;

      path.translate(node.vertex, movement);

      if (node.in >= 0) path.translate(static_cast<size_t>(node.in), movement);
      if (node.out >= 0) path.translate(static_cast<size_t>(node.out), movement);
      if (node.close_vertex >= 0) path.translate(static_cast<size_t>(node.close_vertex), movement);

      return point_index;
    }

    vec2 out_position = position;
    vec2 in_position = 2.0f * path.data().point_at(node.vertex) - position;

    bool swap_in_out = false;

    if (direction) {
      if (*direction == 0) {
        float cos = 0;

        if (node.out >= 0) {
          cos = Math::dot(origin - position, path.data().point_at(static_cast<size_t>(node.out)) - path.data().point_at(static_cast<size_t>(node.vertex)));
        } else if (node.vertex > 0) {
          cos = -Math::dot(origin - position, path.data().point_at(static_cast<size_t>(node.vertex)) - path.data().point_at(static_cast<size_t>(node.vertex - 1)));
        }

        if (cos > 0) *direction = -1;
        else *direction = 1;
      }

      if (*direction < 0) {
        swap_in_out = !swap_in_out;
      }
    }

    if (translate_in_first) {
      swap_in_out = !swap_in_out;

      std::swap(out_position, in_position);
    }

    if (swap_in_out) {
      std::swap(node.in, node.out);
      std::swap(node.in_command, node.out_command);
    }

    size_t new_point_index = point_index;

    if (create_handles && node.out < 0) {
      if (node.out_command < 0) return new_point_index;

      new_point_index = path.to_cubic(static_cast<size_t>(node.out_command), new_point_index);
      node = path.data().node_at(new_point_index);

      if (swap_in_out) {
        std::swap(node.in, node.out);
        std::swap(node.in_command, node.out_command);
      }
    }

    const vec2 old_out_position = path.data().point_at(static_cast<size_t>(node.out));
    const vec2 movement = override_movement ? *override_movement : out_position - old_out_position;

    path.translate(static_cast<size_t>(node.out), movement);

    if (InputManager::keys.alt || (node.in < 0 && (!create_handles || translate_in_first))) {
      return new_point_index;
    }

    if (create_handles && node.in < 0) {
      if (node.in_command < 0) return new_point_index;

      new_point_index = path.to_cubic(static_cast<size_t>(node.in_command), new_point_index);
      node = path.data().node_at(new_point_index);

      if (swap_in_out) {
        std::swap(node.in, node.out);
        std::swap(node.in_command, node.out_command);
      }
    }

    if (keep_in_handle_length) {
      const vec2 vertex_position = path.data().point_at(node.vertex);
      const vec2 dir = Math::normalize(vertex_position - path.data().point_at(static_cast<size_t>(node.out)));

      if (!Math::is_almost_zero(dir)) {
        const float length = Math::distance(path.data().point_at(static_cast<size_t>(node.in)), vertex_position);
        in_position = dir * length + vertex_position;
      } else {
        in_position = path.data().point_at(static_cast<size_t>(node.in));
      }
    }

    path.translate(static_cast<size_t>(static_cast<size_t>(node.in)), in_position - path.data().point_at(static_cast<size_t>(node.in)));

    return new_point_index;
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
              m_cache.push_back(entity.get_component<TransformComponent>());
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

    m_cache.clear();

    update();
  }

  bool Manipulator::on_key(const bool down, const KeyboardKey key) {
    if (!m_active || !m_in_use) return false;

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

    Scene& scene = Editor::scene();
    auto& selected = scene.selection.selected();

    size_t i = 0;

    for (const auto& [id, _] : selected) {
      if (scene.has_entity(id)) {
        Entity entity = scene.get_entity(id);

        if (entity.has_component<TransformComponent>()) {
          TransformComponent transform = entity.get_component<TransformComponent>();

          transform.set(Math::scale(m_cache[i], center, magnitude));
        }
      }

      i++;
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

    Scene& scene = Editor::scene();
    auto& selected = scene.selection.selected();

    size_t i = 0;

    for (const auto& [id, _] : selected) {
      if (scene.has_entity(id)) {
        Entity entity = scene.get_entity(id);

        if (entity.has_component<TransformComponent>()) {
          TransformComponent transform = entity.get_component<TransformComponent>();

          transform.set(Math::rotate(m_cache[i], center, sin_angle, cos_angle));
        }
      }

      i++;
    }
  }

}
