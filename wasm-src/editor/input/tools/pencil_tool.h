#pragma once

#include "../tool.h"
#include "../../../utils/console.h"
#include "../../scene/entities/element_entity.h"
#include "../../../math/models/path_point.h"

class PencilTool: public Tool {
public:
  virtual void on_pointer_down() override;
  virtual void on_pointer_move() override;
  virtual void on_pointer_up(bool abort = false) override;

  virtual void render_overlays(float zoom) const override;
private:
  PencilTool();
  ~PencilTool();
public:
  enum class UpdateStatus {
    Undefined,
    Success,
    FailCorner,
    FailMaxed
  };

  struct Segment {
    vec2 c0, c1, c2, c3;
    std::optional<vec2> constrain_to;
    float error;
    UpdateStatus update_status;
    int steps;
  };

  struct Curve {
    std::vector<Segment> segments;
    std::vector<std::optional<std::vector<std::optional<vec2>>>> vdmap;
  };
private:
  Curve m_curve;
  // std::vector<vec2> m_vector_dist_map;

  std::shared_ptr<ElementEntity> m_entity = nullptr;
  std::vector<PathPoint> m_points;
  std::vector<PathBezier> m_curves;
  // InstancedGeometry m_corners_geo;
private:
  friend class ToolState;
};