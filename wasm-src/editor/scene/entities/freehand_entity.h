#pragma once

#include "../entity.h"
#include "../../input/wobble_smoother.h"
#include "../../../renderer/geometry/stroker.h"
#include "../../../renderer/geometry/bezier_fitter.h"
#include "../../../renderer/geometry/path_simplify.h"
#include "../../../math/models/path_point.h"
#include "element_entity.h"
#include "bezier_entity.h"

#include <vector>

class FreehandEntity: public Entity {
public:
  struct Point {
    vec3 data;
    double time;
  };
public:
  FreehandEntity(vec2 position, float pressure, double time);
  FreehandEntity(const FreehandEntity&) = default;
  FreehandEntity(FreehandEntity&&) = default;

  ~FreehandEntity() {
    console::log("FreehandEntity destroyed");
  }

  inline std::vector<Point>::iterator begin() { return m_points.begin(); }
  inline std::vector<Point>::iterator end() { return m_points.end(); }
  inline std::vector<Point>::const_iterator begin() const { return m_points.begin(); }
  inline std::vector<Point>::const_iterator end() const { return m_points.end(); }

  inline virtual FreehandTransformComponent* transform() override { return &m_transform; }
  inline virtual const FreehandTransformComponent* transform() const override { return &m_transform; }

  inline size_t points_count() const { return m_points.size(); }

  void add_point(vec2 position, float pressure, double time);
  void add_point(vec2 position, float pressure, double time, vec3 updated_data);

  virtual void tessellate_outline(const vec4& color, RenderingOptions options, Geometry& geo) const override;
  virtual void render(RenderingOptions options) const override;

  virtual Entity* entity_at(const vec2& position, bool lower_level, float threshold) override;
  virtual void entities_in(const Box& box, std::vector<Entity*>& entities, bool lower_level) override;
private:
  size_t index_from_t(double t) const;
  Geometry tessellate(RenderingOptions options) const;
private:
  std::vector<Point> m_points;

  FreehandTransformComponent m_transform;
};
