#pragma once

#include "../tool.h"

#include "../../../renderer/geometry/control_point.h"

#include "../../../utils/uuid.h"

#include <memory>
#include <optional>

namespace Graphick::Editor::Input {

  class PenTool : public Tool {
  public:
    virtual void on_pointer_down() override;
    virtual void on_pointer_move() override;
    virtual void on_pointer_up() override;

    virtual void reset() override;

    virtual void render_overlays() const override;
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
      New = 0,
      Join,
      Close,
      Sub,
      Add,
      Angle,
      Start
    };
  private:
    Mode m_mode = Mode::New;
    bool m_reverse = false;

    // TODO: use equivalent of std::nullopt
    History::UUIDValue m_element = uuid{ 0 };
    std::optional<std::weak_ptr<Renderer::Geometry::ControlPoint>> m_vertex = std::nullopt;
  private:
    friend class ToolState;
  };

}
