#pragma once

#include "../entity.h"
#include "../../input/wobble_smoother.h"
#include "../../../renderer/geometry/stroker.h"
#include "../../../renderer/geometry/path_fitter.h"
#include "../../../renderer/geometry/corners_detection.h"

#include <vector>

class FreehandEntity: public Entity {
public:
  FreehandEntity(const vec2& position, float pressure, double time)
    : m_position(position), m_points({ {m_position, pressure} }) {
    console::log("FreehandEntity created");
    WobbleSmoother::reset(m_position, time);
  };
  FreehandEntity(const FreehandEntity&) = default;
  FreehandEntity(FreehandEntity&&) = default;

  ~FreehandEntity() {
    console::log("FreehandEntity destroyed");
  }

  void add_point(const vec2& position, float pressure, double time) {
    vec2 smoothed_position = WobbleSmoother::update(m_position + position, time);

    if (m_points.size() > 1) {
      m_points[m_points.size() - 1].position = smoothed_position;
    }

    m_points.push_back({ m_position + position, pressure });
  }

  virtual void render(float zoom) const override {
    std::vector<FreehandPathPoint> points = smooth_freehand_path(m_points, 4);
    std::vector<Cubic> curves = curve_fit_cubic_to_points(points, 1.0f, 0);

    // Geometry geometry = stroke_freehand_path(points, 8.0f, zoom);

    Geometry geometry{};

    vec2 off_d1{ 1.0f, 1.0f };
    vec2 off_d2{ -1.0f, 1.0f };

    uint32_t offset = 0;

    geometry.vertices.reserve(points.size() * 4);
    geometry.indices.reserve(points.size() * 6);

    for (const FreehandPathPoint& point : points) {
      geometry.vertices.insert(geometry.vertices.end(), {
        {point.position - off_d1, vec4{1.0f, 0.0f, 0.0f, 1.0f}}, point.position + off_d2, point.position + off_d1, point.position - off_d2
        });
      geometry.indices.insert(geometry.indices.end(), {
        offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0
        });

      offset += 4;
    }

    std::vector<uint> corners = detect_corners(points, min_radius, max_radius, max_iterations, min_angle);

    vec4 corner_color{ 0.0f, 1.0f, 0.0f, 1.0f };

    for (uint corner : corners) {
      geometry.vertices.insert(geometry.vertices.end(), {
        {points[corner].position - off_d1, corner_color}, {points[corner].position + off_d2, corner_color},
        {points[corner].position + off_d1, corner_color}, {points[corner].position - off_d2, corner_color}
        });
      geometry.indices.insert(geometry.indices.end(), {
        offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0
        });

      offset += 4;
    }

    // for (Cubic& cubic : curves) {
    //   for (int i = 0; i < 10; i++) {
    //     float t = (float)i / 10.0f;

    //     float c0 = std::powf(1.0f - t, 3.0f);
    //     float c1 = 3.0f * t * std::powf(1.0f - t, 2.0f);
    //     float c2 = 3.0f * std::powf(t, 2.0f) * (1.0f - t);
    //     float c3 = std::powf(t, 3.0f);

    //     vec2 pt = cubic.p0 * c0 + cubic.p1 * c1 + cubic.p2 * c2 + cubic.p3 * c3;

    //     geometry.vertices.insert(geometry.vertices.end(), {
    //       pt - off_d1, pt + off_d2, pt + off_d1,pt - off_d2
    //       });
    //     geometry.indices.insert(geometry.indices.end(), {
    //       offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0
    //       });

    //     offset += 4;
    //   }

    //   // geometry.vertices.insert(geometry.vertices.end(), { 
    //   //   cubic.p0 - off_d1, cubic.p0 + off_d2, cubic.p0 + off_d1,cubic.p0 - off_d2, 
    //   //   cubic.p3 - off_d1, cubic.p3 + off_d2, cubic.p3 + off_d1,cubic.p3 - off_d2
    //   // });

    //   // geometry.indices.insert(geometry.indices.end(), {
    //   //   offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0,
    //   //   offset + 4, offset + 5, offset + 6, offset + 6, offset + 7, offset + 4
    //   // });

    //   // offset += 8;
    // }

    Renderer::get()->draw(geometry);
  };
private:
  vec2 m_position;
  std::vector<FreehandPathPoint> m_points;
};
