#pragma once

#include "../../math/vec2.h"

#include "../../utils/uuid.h"

#include <optional>
#include <memory>

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

    ~HoverState();

    inline HoverType type() const { return m_type; }
    inline uuid entity_id() const { return m_entity; }

    std::optional<Entity> entity() const;
    std::optional<size_t> segment() const;
    std::optional<size_t> vertex() const;
    std::optional<size_t> handle() const;
  private:
    void set_hovered(const uuid entity, const vec2 position, const bool deep_search, float threshold, const double zoom);
    void reset();
  private:
    HoverType m_type = HoverType::None;

    uuid m_entity = uuid::null;
    int64_t m_segment = -1;
    int64_t m_vertex = -1;
    int64_t m_handle = -1;
  private:
    friend class InputManager;
  };

}
