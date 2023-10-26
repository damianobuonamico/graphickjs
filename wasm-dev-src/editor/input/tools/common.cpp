#include "common.h"

#include "../../editor.h"
#include "../../scene/entity.h"

#include "../../../history/values.h"

#include "../../../math/math.h"

#include "../../../utils/console.h"

namespace Graphick::Editor::Input {

  /* -- SelectionRect -- */

  SelectionRect::SelectionRect(bool dashed) :
    m_position({ 0.0f, 0.0f }), m_anchor_position({ 0.0f, 0.0f }),
    m_dashed(dashed)
  {
    m_path.move_to({ 0.0f, 0.0f });
    m_path.line_to({ 0.0f, 0.0f });
    m_path.line_to({ 0.0f, 0.0f });
    m_path.line_to({ 0.0f, 0.0f });
    m_path.line_to({ 0.0f, 0.0f });
    m_path.close();
  }

  Math::rect SelectionRect::bounding_rect() const {
    return m_path.bounding_rect() + m_position;
  }

  void SelectionRect::set(const vec2 position) {
    m_anchor_position = m_position = position;
    m_active = true;

    size({ 0.0f, 0.0f });
  }

  void SelectionRect::size(const vec2 size) {
    const auto& segments = m_path.segments();

    if (segments.size() < 4) return;

    vec2 new_size = size;

    m_position = m_anchor_position;

    if (size.x < 0) {
      m_position.x = m_anchor_position.x + size.x;
      new_size.x = -size.x;
    }
    if (size.y < 0) {
      m_position.y = m_anchor_position.y + size.y;
      new_size.y = -size.y;
    }

    *segments[0].p3_ptr().lock() = { new_size.x, 0.0f };
    *segments[1].p3_ptr().lock() = new_size;
    *segments[2].p3_ptr().lock() = { 0.0f, new_size.y };
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

  bool Manipulator::on_pointer_down(const vec2 position, const float threshold) {
    m_cache.clear();

    if (!m_active) {
      m_active_handle = HandleNone;
      m_in_use = false;

      return false;
    }

    for (int i = 0; i < HandleNone; i++) {
      if (Math::is_point_in_circle(position, m_handles[i], i >= 8 ? threshold * 2.0f : threshold)) {
        m_active_handle = static_cast<HandleType>(i);
        m_in_use = true;

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
        }

        m_start_bounding_rect = bounding_rect();
        m_handle = m_handles[i];

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

  void Manipulator::on_pointer_move(const vec2 position) {
    if (!m_active || m_active_handle > SW) return;

    vec2 old_delta = m_handle - m_center;
    vec2 delta = position - m_center;

    vec2 magnitude = delta / old_delta;

    if (m_active_handle == N || m_active_handle == S) {
      magnitude.x = 1.0f;
      delta.x = old_delta.x;
    } else if (m_active_handle == E || m_active_handle == W) {
      magnitude.y = 1.0f;
      delta.y = old_delta.y;
    }

    rect new_bounding_rect = {
      Math::scale(m_start_bounding_rect.min, m_center, magnitude),
      Math::scale(m_start_bounding_rect.max, m_center, magnitude)
    };

    update_positions(new_bounding_rect);

    for (History::Mat2x3Value* matrix : m_cache) {
      matrix->set_delta(mat2x3{ 0.0f });
      matrix->scale(m_center, magnitude);
    }
  }

  void Manipulator::on_pointer_up() {
    m_active_handle = HandleNone;
    m_in_use = false;

    for (History::Mat2x3Value* matrix : m_cache) {
      matrix->apply();
    }

    m_cache.clear();
  }

  void Manipulator::update_positions(const rect& bounding_rect) {
    set(bounding_rect.min);
    size(bounding_rect.size());

    float x = bounding_rect.min.x;
    float y = bounding_rect.min.y;
    float w = bounding_rect.size().x;
    float h = bounding_rect.size().y;
    float h2 = h / 2.0f;
    float w2 = w / 2.0f;

    m_handles[N] = m_handles[RN] = { x + w2, y };
    m_handles[S] = m_handles[RS] = { x + w2, y + h };
    m_handles[E] = m_handles[RE] = { x + w, y + h2 };
    m_handles[W] = m_handles[RW] = { x, y + h2 };

    m_handles[NW] = m_handles[RNW] = { x, y };
    m_handles[NE] = m_handles[RNE] = { x + w, y };
    m_handles[SE] = m_handles[RSE] = { x + w, y + h };
    m_handles[SW] = m_handles[RSW] = { x, y + h };
  }

}
