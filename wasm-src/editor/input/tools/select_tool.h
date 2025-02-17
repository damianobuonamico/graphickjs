/**
 * @file select_tool.h
 * @brief Contains the declaration of the SelectTool class.
 */

#pragma once

#include "common.h"

#include "../tool.h"

#include "../../../utils/uuid.h"

#include <optional>

namespace graphick::editor::input {

/**
 * @brief The SelectTool class represents a tool used for selecting entities.
 *
 * This is the default tool in every editor mode.
 */
class SelectTool : public Tool {
 public:
  virtual void on_pointer_down() override;
  virtual void on_pointer_move() override;
  virtual void on_pointer_up() override;

  virtual void render_overlays() const override;

 private:
  /**
   * @brief Default constructor.
   */
  SelectTool();

 private:
  bool m_is_entity_added_to_selection =
      false;                         // Whether the entity was just added to the selection.
  bool m_dragging_occurred = false;  // Whether a dragging occurred.

  uuid m_entity = uuid::null;        // The UUID of the active entity.

  SelectionRect m_selection_rect;    // The selection rectangle.
 private:
  friend class ToolState;
};

}  // namespace graphick::editor::input
