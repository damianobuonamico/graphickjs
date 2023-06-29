#pragma once

#include "../tool.h"
// #include "../../scene/entities/selection_rect_entity.h"

namespace Graphick::Editor::Input {

  class SelectTool : public Tool {
  public:
    virtual void on_pointer_down() override;
    virtual void on_pointer_move() override;
    virtual void on_pointer_up() override;

    // virtual void tessellate_overlays_outline(const vec4& color, const RenderingOptions& options, Geometry& geo) const override;
    // virtual void render_overlays(const RenderingOptions& options) const override;
  private:
    SelectTool();
  private:
    bool m_dragging_occurred = false;
    bool m_is_element_added_to_selection = false;
    // Entity* m_element = nullptr;

    // SelectionRectEntity m_selection_rect;
  private:
    friend class ToolState;
  };

}
