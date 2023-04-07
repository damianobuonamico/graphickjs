#pragma once

#include "../entity.h"
#include "../../../utils/cache.h"

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

  virtual void tessellate_outline(const vec4& color, const RenderingOptions& options, Geometry& geo) const override;
  virtual void render(const RenderingOptions& options) const override;

  virtual Entity* entity_at(const vec2& position, bool lower_level, float threshold) override;
  virtual void entities_in(const Box& box, std::vector<Entity*>& entities, bool lower_level) override;
private:
  size_t index_from_t(double t) const;
  Geometry tessellate(const RenderingOptions& options) const;
private:
  std::vector<Point> m_points;

  mutable Cached<Geometry> m_geometry;

  FreehandTransformComponent m_transform;
};
