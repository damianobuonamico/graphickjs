#pragma once

#include "common.h"

#include "../tool.h"

#include "../../../renderer/geometry/control_point.h"

#include "../../../utils/uuid.h"

#include <memory>
#include <optional>

namespace Graphick::Editor::Input {

  class DirectSelectTool : public Tool {
  public:
    virtual void on_pointer_down() override;
    virtual void on_pointer_move() override;
    virtual void on_pointer_up() override;

    virtual void on_key(const bool down, const KeyboardKey key) override;

    virtual void render_overlays() const override;
  private:
    DirectSelectTool();

    void populate_cache();
    void translate_selected();
    void apply_selected();

    void on_none_pointer_down();
    void on_duplicate_pointer_down();
    void on_entity_pointer_down();
    void on_element_pointer_down();
    void on_bezier_pointer_down();
    void on_vertex_pointer_down();
    void on_handle_pointer_down();

    void on_none_pointer_move();
    void on_duplicate_pointer_move();
    void on_entity_pointer_move();
    void on_element_pointer_move();
    void on_bezier_pointer_move();
    void on_vertex_pointer_move();
    void on_handle_pointer_move();

    void on_none_pointer_up();
    void on_duplicate_pointer_up();
    void on_entity_pointer_up();
    void on_element_pointer_up();
    void on_bezier_pointer_up();
    void on_vertex_pointer_up();
    void on_handle_pointer_up();
  private:
    enum class Mode {
      None = 0,
      Duplicate,
      Element,
      Vertex,
      Handle,
      Bezier,
      Entity
    };
  private:
    bool m_dragging_occurred = false;
    bool m_is_entity_added_to_selection = false;
    bool m_should_evaluate_selection = false;
    // vec2 m_last_bezier_point{};
    // vec2 m_last_bezier_p1{};
    // vec2 m_last_bezier_p2{};
    // BezierEntity::BezierPointDistance m_closest{};

    Mode m_mode = Mode::None;

    uuid m_entity = uuid::null;
    std::optional<std::weak_ptr<Renderer::Geometry::ControlPoint>> m_vertex = std::nullopt;
    std::optional<std::weak_ptr<History::Vec2Value>> m_handle = std::nullopt;

    std::vector<History::Vec2Value*> m_vector_cache;
    std::vector<History::Mat2x3Value*> m_matrix_cache;

    SelectionRect m_selection_rect;
  private:
    friend class ToolState;
  };

}
