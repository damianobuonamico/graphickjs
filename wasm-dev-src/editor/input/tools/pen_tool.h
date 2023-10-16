#pragma once

#include "../tool.h"

#include "../../../utils/uuid.h"

#include <memory>
#include <optional>

namespace Graphick::Renderer::Geometry {
  class ControlPoint;
  class Path;
}

namespace Graphick::Editor::Input {

  class PenTool : public Tool {
  public:
    virtual void on_pointer_down() override;
    virtual void on_pointer_move() override;
    virtual void on_pointer_up() override;

    virtual void reset() override;

    virtual void render_overlays() const override;

    inline uuid pen_element() const { return m_element; }
    inline void set_pen_element(const uuid id) { m_element = id; }
  private:
    PenTool();

    void on_new_pointer_down();
    void on_join_pointer_down();
    void on_close_pointer_down();
    void on_sub_pointer_down();
    void on_add_pointer_down();
    void on_angle_pointer_down();
    void on_start_pointer_down();

    void on_new_pointer_move();
    void on_join_pointer_move();
    void on_close_pointer_move();
    void on_sub_pointer_move();
    void on_add_pointer_move();
    void on_angle_pointer_move();
    void on_start_pointer_move();

    void on_new_pointer_up();
    void on_join_pointer_up();
    void on_close_pointer_up();
    void on_sub_pointer_up();
    void on_add_pointer_up();
    void on_angle_pointer_up();
    void on_start_pointer_up();
  private:
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
    Mode m_mode = Mode::New;
    // TODO: use equivalent of std::nullopt
    uuid m_element = 0;

    Renderer::Geometry::ControlPoint* m_vertex = nullptr;
    Renderer::Geometry::Path* m_path = nullptr;

    int m_add_direction = 0;
  private:
    friend class ToolState;
  };

}
