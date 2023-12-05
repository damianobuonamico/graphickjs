#pragma once

#include "../../history/values.h"

namespace Graphick::Renderer::Geometry {

  class ControlPoint : public History::Vec2Value {
  public:
    uuid id;
  public:
    ControlPoint() : History::Vec2Value() {}
    ControlPoint(const vec2 value) : History::Vec2Value(value) {}
    ControlPoint(float x, float y) : History::Vec2Value(x, y) {}
    ~ControlPoint() = default;
  };

}
