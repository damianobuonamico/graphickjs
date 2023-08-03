#pragma once

#include "../../history/values.h"

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

    std::optional<Entity> entity() const;
    std::optional<std::weak_ptr<History::Vec2Value>> vertex() const;
    std::optional<std::weak_ptr<History::Vec2Value>> handle() const;
  private:
    void set_hovered(const uuid entity, const vec2 position, bool lower_level, float threshold);
    void reset();
  private:
    HoverType m_type = HoverType::None;

    uuid m_entity = 0;
    std::optional<std::weak_ptr<History::Vec2Value>> m_vertex;
    std::optional<std::weak_ptr<History::Vec2Value>> m_handle;
  private:
    friend class InputManager;
  };

}
