#include "hover_state.h"

#include "../scene/entity.h"
#include "../editor.h"

#include "../../math/math.h"
#include "../../math/matrix.h"

#include "../../utils/console.h"

namespace Graphick::Editor::Input {

  HoverState::~HoverState() {}

  std::optional<Entity> HoverState::entity() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return Editor::scene().get_entity(m_entity);
  }

  std::optional<size_t> HoverState::segment() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return static_cast<size_t>(m_segment);
  }

  std::optional<size_t> HoverState::vertex() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return static_cast<size_t>(m_vertex);
  }

  std::optional<size_t> HoverState::handle() const {
    if (m_entity == 0 || !Editor::scene().has_entity(m_entity)) return std::nullopt;
    return static_cast<size_t>(m_handle);
  }

  void HoverState::set_hovered(const uuid id, const vec2 position, const bool deep_search, float threshold, const double zoom) {
    reset();

    const Scene& scene = Editor::scene();
    m_entity = id;

    if (id == uuid::null || !scene.has_entity(id)) {
      console::log("HoverState::set_hovered: none.");
      m_type = HoverType::None;
      return;
    }

    const Entity entity = scene.get_entity(id);

    if (!entity.is_element()) {
      console::log("HoverState::set_hovered: entity.");

      m_type = HoverType::Entity;
      return;
    }

    m_type = HoverType::Element;

    if (!deep_search) {
      return;
    }

    const PathComponent path = entity.get_component<PathComponent>();
    const TransformComponent transform = entity.get_component<TransformComponent>();

    threshold /= zoom;

    // const vec2 transformed_pos = transform / position;
    // const vec2 transformed_threshold = vec2{ threshold } / Math::decompose(transform).scale;
    // const float mean_threshold = (transformed_threshold.x + transformed_threshold.y) / 2.0f;

    bool deep = deep_search && scene.selection.has(id);

    if (path.data().empty()) {
      if (path.data().vacant()) {
        m_type = HoverType::None;
        m_entity = 0;
        return;
      }

      // auto last = path.last();

      // if (Math::is_point_in_ellipse(transformed_pos, last.lock()->get(), transformed_threshold)) {
      //   m_type = HoverType::Vertex;
      //   m_vertex = last;
      //   return;
      // }
    } else {
      console::log("size", path.data().size());
      for (size_t i = 0; i < path.data().size(); i++) {
        if (path.data().is_point_inside_segment(i, position, nullptr, transform, threshold, zoom)) {
          console::log("HoverState::set_hovered: segment.");
          m_type = HoverType::Segment;
          m_segment = i;
          m_vertex = -1;
          m_handle = -1;
          return;
        }
      }

    // for (const auto& segment : path.segments()) {
    //   vec2 p0 = segment->p0();

    //   if (Math::is_point_in_ellipse(transformed_pos, p0, transformed_threshold)) {
    //     m_type = HoverType::Vertex;
    //     m_vertex = segment->p0_ptr();
    //     return;
    //   }

    //   vec2 p3 = segment->p3();

    //   if (deep && (segment->is_quadratic() || segment->is_cubic())) {
    //     vec2 p1 = segment->p1();
    //     vec2 p2 = segment->p2();

    //     if (segment->has_p1() && Math::is_point_in_ellipse(transformed_pos, p1, transformed_threshold)) {
    //       m_type = HoverType::Handle;
    //       m_vertex = segment->p0_ptr();
    //       m_handle = segment->p1_ptr();
    //       return;
    //     }

    //     if (segment->is_cubic()) {
    //       if (segment->has_p2() && Math::is_point_in_ellipse(transformed_pos, p2, transformed_threshold)) {
    //         m_type = HoverType::Handle;
    //         m_vertex = segment->p3_ptr();
    //         m_handle = segment->p2_ptr();
    //         return;
    //       }
    //     }
    //   }

    //   if (Math::is_point_in_ellipse(transformed_pos, p3, transformed_threshold)) {
    //     m_type = HoverType::Vertex;
    //     m_vertex = segment->p3_ptr();
    //     m_handle.reset();
    //     return;
    //   }

    //   auto closest = segment->closest_to(transformed_pos);

    //   /* Adjusted threshold for segment hover. */
    //   if (closest.sq_distance <= mean_threshold * mean_threshold / 3.0f) {
    //     m_type = HoverType::Segment;
    //     m_segment = std::make_pair(std::weak_ptr<Renderer::Geometry::Segment>(segment), closest.t);
    //     m_vertex.reset();
    //     m_handle.reset();
    //     return;
    //   }
    // }
    }

    // auto in_handle = path.in_handle_ptr();
    // auto out_handle = path.out_handle_ptr();

    // if (in_handle) {
    //   if (Math::is_point_in_circle(transformed_pos, in_handle.value()->get(), threshold)) {
    //     m_type = HoverType::Handle;
    //     m_vertex = path.empty() ? path.last() : path.segments().front().p0_ptr();
    //     m_handle = path.in_handle_ptr();
    //     return;
    //   }
    // }
    // if (out_handle) {
    //   if (Math::is_point_in_circle(transformed_pos, out_handle.value()->get(), threshold)) {
    //     m_type = HoverType::Handle;
    //     m_vertex = path.empty() ? path.last() : path.segments().back().p3_ptr();
    //     m_handle = path.out_handle_ptr();
    //     return;
    //   }
    // }

    // m_type = HoverType::Element;
    console::log("HoverState::set_hovered: element.");
  }

  void HoverState::reset() {
    m_type = HoverType::None;
    m_entity = uuid::null;
    m_segment = -1;
    m_vertex = -1;
    m_handle = -1;
  }

}
