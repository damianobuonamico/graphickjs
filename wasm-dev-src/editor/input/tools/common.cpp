#include "common.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../history/values.h"

#include "../../../math/math.h"
#include "../../../math/matrix.h"

#include "../../../utils/console.h"

namespace Graphick::Editor::Input {

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

  // void SelectionRect::size(const vec2 size) {
  //   const auto& segments = m_path.segments();

  //   if (segments.size() < 4) return;

  //   vec2 new_size = size;

  //   m_position = m_anchor_position;

  //   if (size.x < 0) {
  //     m_position.x = m_anchor_position.x + size.x;
  //     new_size.x = -size.x;
  //   }
  //   if (size.y < 0) {
  //     m_position.y = m_anchor_position.y + size.y;
  //     new_size.y = -size.y;
  //   }

  //   *segments[0].p3_ptr().lock() = { new_size.x, 0.0f };
  //   *segments[1].p3_ptr().lock() = new_size;
  //   *segments[2].p3_ptr().lock() = { 0.0f, new_size.y };
  // }

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

  bool Manipulator::on_pointer_down(const vec2 position, const float threshold) {
    m_start_transform = transform();
    m_threshold = threshold;
    m_cache.clear();

    if (!m_active) {
      m_active_handle = HandleNone;
      m_in_use = false;

      return false;
    }

    vec2 transformed_position = transform() / position;
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
              m_cache.push_back(entity.get_component<TransformComponent>()._value());
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

  void Manipulator::on_pointer_move(const vec2 position, const bool shift) {
    if (!m_active) return;

    if (m_active_handle > SW) on_rotate_pointer_move(position, shift);
    else on_scale_pointer_move(position, shift);
  }

  void Manipulator::on_pointer_up() {
    m_active_handle = HandleNone;
    m_in_use = false;

    for (History::Mat2x3Value* matrix : m_cache) {
      matrix->apply();
    }

    m_cache.clear();

    update();
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

  void Manipulator::on_scale_pointer_move(const vec2 position, const bool shift) {
    vec2 old_delta = m_handle - m_center;
    vec2 delta = m_start_transform / position - m_center;

    vec2 magnitude = delta / old_delta;
    uint8_t axial = 0;    /* 0 = none, 1 = x, 2 = y */

    if (m_active_handle == N || m_active_handle == S) {
      magnitude.x = 1.0f;
      axial = 1;
    } else if (m_active_handle == E || m_active_handle == W) {
      magnitude.y = 1.0f;
      axial = 2;
    }

    if (shift) {
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

    vec2 center = m_start_transform * m_center;

    rrect new_bounding_rect = {
      Math::scale(m_start_bounding_rect.min, center, magnitude),
      Math::scale(m_start_bounding_rect.max, center, magnitude)
    };

    update_positions(new_bounding_rect);

    for (History::Mat2x3Value* matrix : m_cache) {
      matrix->set_delta(mat2x3{ 0.0f });
      matrix->scale(center, magnitude);
    }
  }

  void Manipulator::on_rotate_pointer_move(const vec2 position, const bool shift) {
    float angle = Math::angle(m_handle - m_center, m_start_transform / position - m_center);
    float sin_angle = std::sinf(angle);
    float cos_angle = std::cosf(angle);

    vec2 center = m_start_transform * m_center;

    rrect new_bounding_rect = {
      m_start_bounding_rect,
      angle
    };

    update_positions(new_bounding_rect);

    for (History::Mat2x3Value* matrix : m_cache) {
      matrix->set_delta(mat2x3{ 0.0f });
      matrix->rotate(center, sin_angle, cos_angle);
    }
  }

}
