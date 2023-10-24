/**
 * @file pen_tool.h
 * @brief Contains the declaration of the PenTool class.
 */

#pragma once

#include "../tool.h"

#include "../../../utils/uuid.h"

#include <memory>
#include <optional>

namespace Graphick::Renderer::Geometry {
  class ControlPoint;
  class Path;
}

namespace Graphick::History {
  class Mat2x3Value;
}

namespace Graphick::Editor::Input {

  /**
   * @brief The PenTool class represents a tool used for drawing paths with a pen.
   *
   * This class inherits from the Tool class and implements its virtual methods.
   * It also provides additional methods for setting and getting the UUID of the pen element.
   * The class has a private Mode enum for keeping track of the current mode of the tool.
   *
   * @class PenTool
   */
  class PenTool : public Tool {
  public:
    virtual void on_pointer_down() override;
    virtual void on_pointer_move() override;
    virtual void on_pointer_up() override;

    virtual void reset() override;

    virtual void render_overlays() const override;

    /**
     * @brief Returns the UUID of the pen element.
     *
     * @return The UUID of the pen element.
     */
    inline uuid pen_element() const { return m_element; }

    /**
     * @brief Sets the pen element to the specified UUID.
     *
     * @param id The UUID of the pen element to set.
     */
    void set_pen_element(const uuid id);
  private:
    PenTool();

    /**
     * @brief Adds a new point to an existing path or creates a new path.
     */
    void on_new_pointer_down();

    /**
     * @brief Joins the active path to the current path.
     */
    void on_join_pointer_down();

    /**
     * @brief Closes the active path.
     */
    void on_close_pointer_down();

    /**
     * @brief Subdivides the active path at the specified point.
     *
     * If the shift key is pressed, the overall shape of the path is preserved.
     */
    void on_sub_pointer_down();

    /**
     * @brief Splits the active path at the specified point.
     */
    void on_add_pointer_down();

    /**
     * @brief Edits the handles of the active vertex.
     */
    void on_angle_pointer_down();

    /**
     * @brief Sets the active path and vertex.
     */
    void on_start_pointer_down();
  private:
    /**
     * @brief Enum class representing the different modes of the pen tool.
     */
    enum class Mode {
      None = 0,
      New,
      Join,
      Close,
      Sub,
      Add,
      Angle,
      Start
    };
  private:
    Mode m_mode = Mode::New;                                /* The current mode of the pen tool. */
    uuid m_element = uuid::null;                            /* The UUID of the pen element. */
    History::Mat2x3Value* m_transform = nullptr;            /* The transform of the pen element. */

    Renderer::Geometry::ControlPoint* m_vertex = nullptr;   /* The active vertex. */
    Renderer::Geometry::Path* m_path = nullptr;             /* The active path. */

    int m_direction = 0;                                    /* The direction of the active path. */
  private:
    friend class ToolState;
  };

}
