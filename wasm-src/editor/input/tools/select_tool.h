#pragma once

#include "common.h"

#include "../tool.h"

#include "../../../utils/uuid.h"

#include <optional>

namespace Graphick::Editor::Input {

  class SelectTool : public Tool {
  public:
    virtual void on_pointer_down() override;
    virtual void on_pointer_move() override;
    virtual void on_pointer_up() override;

    virtual void render_overlays() const override;
  private:
    SelectTool();
  private:
    bool m_is_entity_added_to_selection = false;
    bool m_dragging_occurred = false;
    uuid m_entity = uuid::null;

    SelectionRect m_selection_rect;
  private:
    friend class ToolState;
  };

}
