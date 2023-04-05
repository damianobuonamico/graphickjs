#pragma once

#include "../tool.h"
#include "../../../utils/console.h"
#include "../../scene/entities/freehand_entity.h"
#include "../../../math/models/path_point.h"

#define SPRING_FREEHAND

class PencilTool: public Tool {
public:
  virtual void on_pointer_down() override;
  virtual void on_pointer_move() override;
  virtual void on_pointer_up(bool abort = false) override;

  virtual void render_overlays(RenderingOptions options) const override;
private:
  PencilTool();
  ~PencilTool();
public:
#ifndef SPRING_FREEHAND
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
#endif
private:
#ifdef SPRING_FREEHAND
  std::shared_ptr<FreehandEntity> m_entity = nullptr;
#else
  Curve m_curve;
  // std::vector<vec2> m_vector_dist_map;

  std::shared_ptr<ElementEntity> m_entity = nullptr;
  std::vector<PathPoint> m_points;
  std::vector<PathBezier> m_curves;
  // InstancedGeometry m_corners_geo;
#endif
private:
  friend class ToolState;
};