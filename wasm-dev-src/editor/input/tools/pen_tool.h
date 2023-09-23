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

    void on_none_pointer_down();

    void on_none_pointer_move();

    void on_none_pointer_up();
  private:
    enum class Mode {
      None = 0,
    };
  private:
    Mode m_mode = Mode::None;

    uuid m_element = 0;
    // std::optional<std::weak_ptr<Renderer::Geometry::ControlPoint>> m_vertex = std::nullopt;
  private:
    friend class ToolState;
  };

}
