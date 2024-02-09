#include "hover_state.h"

#include "../scene/entity.h"
#include "../editor.h"

#include "../../renderer/geometry/control_point.h"
#include "../../renderer/geometry/segment.h"

#include "../../history/values.h"

#include "../../math/math.h"
#include "../../math/matrix.h"

namespace Graphick::Editor::Input {

  HoverState::~HoverState() {}

  std::optional<Entity> HoverState::entity() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return Editor::scene().get_entity(m_entity);
  }

  std::optional<std::pair<std::weak_ptr<Renderer::Geometry::Segment>, float>> HoverState::segment() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return m_segment;
  }

  std::optional<std::weak_ptr<Renderer::Geometry::ControlPoint>> HoverState::vertex() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return m_vertex;
  }

  std::optional<std::weak_ptr<Graphick::History::Vec2Value>> HoverState::handle() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return m_handle;
  }

  void HoverState::set_hovered(const uuid id, const vec2 position, bool deep_search, float threshold) {
    reset();

    Scene& scene = Editor::scene();
    m_entity = id;

    if (id == 0 || !scene.has_entity(id)) {
      m_type = HoverType::None;
      return;
    }

    Entity entity = scene.get_entity(id);

    if (!entity.is_element()) {
      m_type = HoverType::Entity;
      return;
    }

    if (!deep_search) {
      m_type = HoverType::Element;
      return;
    }

#if 0
    const Renderer::Geometry::Path& path = entity.get_component<PathComponent>().path;
    const mat2x3 transform = entity.get_component<TransformComponent>().get();
    const vec2 transformed_pos = transform / position;
    const vec2 transformed_threshold = vec2{ threshold } / Math::decompose(transform).scale;
    const float mean_threshold = (transformed_threshold.x + transformed_threshold.y) / 2.0f;

    bool deep = deep_search && scene.selection.has(id);

    if (path.empty()) {
      if (path.vacant()) {
        m_entity = 0;
        m_type = HoverType::None;
        return;
      }

      auto last = path.last();

      if (Math::is_point_in_ellipse(transformed_pos, last.lock()->get(), transformed_threshold)) {
        m_type = HoverType::Vertex;
        m_vertex = last;
        return;
      }
    } else {
      for (const auto& segment : path.segments()) {
        vec2 p0 = segment->p0();

        if (Math::is_point_in_ellipse(transformed_pos, p0, transformed_threshold)) {
          m_type = HoverType::Vertex;
          m_vertex = segment->p0_ptr();
          return;
        }

        vec2 p3 = segment->p3();

        if (deep && (segment->is_quadratic() || segment->is_cubic())) {
          vec2 p1 = segment->p1();
          vec2 p2 = segment->p2();

          if (segment->has_p1() && Math::is_point_in_ellipse(transformed_pos, p1, transformed_threshold)) {
            m_type = HoverType::Handle;
            m_vertex = segment->p0_ptr();
            m_handle = segment->p1_ptr();
            return;
          }

          if (segment->is_cubic()) {
            if (segment->has_p2() && Math::is_point_in_ellipse(transformed_pos, p2, transformed_threshold)) {
              m_type = HoverType::Handle;
              m_vertex = segment->p3_ptr();
              m_handle = segment->p2_ptr();
              return;
            }
          }
        }

        if (Math::is_point_in_ellipse(transformed_pos, p3, transformed_threshold)) {
          m_type = HoverType::Vertex;
          m_vertex = segment->p3_ptr();
          m_handle.reset();
          return;
        }

        auto closest = segment->closest_to(transformed_pos);

        /* Adjusted threshold for segment hover. */
        if (closest.sq_distance <= mean_threshold * mean_threshold / 3.0f) {
          m_type = HoverType::Segment;
          m_segment = std::make_pair(std::weak_ptr<Renderer::Geometry::Segment>(segment), closest.t);
          m_vertex.reset();
          m_handle.reset();
          return;
        }
      }
    }

    auto in_handle = path.in_handle_ptr();
    auto out_handle = path.out_handle_ptr();

    if (in_handle) {
      if (Math::is_point_in_circle(transformed_pos, in_handle.value()->get(), threshold)) {
        m_type = HoverType::Handle;
        m_vertex = path.empty() ? path.last() : path.segments().front().p0_ptr();
        m_handle = path.in_handle_ptr();
        return;
      }
    }
    if (out_handle) {
      if (Math::is_point_in_circle(transformed_pos, out_handle.value()->get(), threshold)) {
        m_type = HoverType::Handle;
        m_vertex = path.empty() ? path.last() : path.segments().back().p3_ptr();
        m_handle = path.out_handle_ptr();
        return;
      }
    }

    m_type = HoverType::Element;
#endif
  }

  void HoverState::reset() {
    m_type = HoverType::None;
    m_entity = 0;
    m_segment.reset();
    m_vertex.reset();
    m_handle.reset();
  }

}
