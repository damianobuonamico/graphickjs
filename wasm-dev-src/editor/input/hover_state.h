#pragma once

#include "../../renderer/geometry/control_point.h"
#include "../../renderer/geometry/segment.h"

#include "../../utils/uuid.h"

#include <optional>

namespace Graphick::Editor {
  class Entity;
}

namespace Graphick::Editor::Input {

  class HoverState {
  public:
    enum class HoverType {
      None = 0,
      Entity,
      Element,
      Segment,
      Vertex,
      Handle
    };
  public:
    HoverState() = default;

    HoverState(const HoverState&) = delete;
    HoverState(HoverState&&) = delete;

    ~HoverState() = default;

    inline HoverType type() const { return m_type; }
    inline uuid entity_id() const { return m_entity; }

    std::optional<Entity> entity() const;
    std::optional<std::pair<std::weak_ptr<Renderer::Geometry::Segment>, float>> segment() const;
    std::optional<std::weak_ptr<Renderer::Geometry::ControlPoint>> vertex() const;
    std::optional<std::weak_ptr<History::Vec2Value>> handle() const;
  private:
    void set_hovered(const uuid entity, const vec2 position, bool deep_search, float threshold);
    void reset();
  private:
    HoverType m_type = HoverType::None;

    uuid m_entity = 0;
    std::optional<std::pair<std::weak_ptr<Renderer::Geometry::Segment>, float>> m_segment = std::nullopt;
    std::optional<std::weak_ptr<Renderer::Geometry::ControlPoint>> m_vertex = std::nullopt;
    std::optional<std::weak_ptr<History::Vec2Value>> m_handle = std::nullopt;
  private:
    friend class InputManager;
  };

}
