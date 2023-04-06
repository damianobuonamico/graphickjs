#include "pencil_tool.h"

#include "../../editor.h"
#include "../input_manager.h"
#include "../wobble_smoother.h"
#include "../../../math/models/corner_detector.h"
#include "../../../math/models/path_fitter.h"
#include "../../../math/mat3.h"
#include "../../../math/matrix.h"
#include "../../settings.h"

PencilTool::PencilTool(): Tool(ToolType::Pencil, CategoryImmediate | CategoryDirect) {}

#ifdef SPRING_FREEHAND

void PencilTool::on_pointer_down() {
  float pressure = std::max(InputManager::pointer.pressure, 0.1f);

  m_entity = std::make_shared<FreehandEntity>(InputManager::pointer.scene.position, pressure, InputManager::pointer.time);

  WobbleSmoother::reset(
    { InputManager::pointer.type != InputManager::PointerType::Pen, 20.0f, 40.0f, 1.31f, 1.44f },
    vec2{ 0.0f }, pressure, InputManager::pointer.time
  );

  Editor::scene.add_entity(m_entity);
}

void PencilTool::on_pointer_move() {
  if (!m_entity) return;

  float pressure = 0.5f + (InputManager::pointer.pressure - 0.5f) * 0.8f;

  vec3 smoothed_point = WobbleSmoother::update(InputManager::pointer.scene.delta, std::max(pressure, 0.1f), InputManager::pointer.time);
  m_entity->add_point(InputManager::pointer.scene.delta, pressure, InputManager::pointer.time, smoothed_point);
}

void PencilTool::on_pointer_up(bool abort) {
  m_entity = nullptr;
}

void PencilTool::render_overlays(RenderingOptions options) const {
  // if (!m_entity) return;
  // m_entity->render(zoom);
}

#else

void PencilTool::on_pointer_down() {
  m_points.clear();
  m_curves.clear();

  if (m_entity) {
    Editor::scene.selection.deselect(m_entity->id);
  }

  m_entity = std::make_shared<ElementEntity>(InputManager::pointer.scene.position);
  m_points.push_back({ vec2{ 0.0f }, /*InputManager::pointer.pressure*/1.0f });

  WobbleSmoother::reset(
    { InputManager::pointer.type != InputManager::PointerType::Pen, 20.0f, 40.0f, 1.31f, 1.44f },
    vec2{ 0.0f }, /*InputManager::pointer.pressure*/1.0f, InputManager::pointer.time
  );
}

void PencilTool::on_pointer_move() {
  if (!m_entity) return;

  vec3 smoothed_point = WobbleSmoother::update(InputManager::pointer.scene.delta, /*InputManager::pointer.pressure*/1.0f, InputManager::pointer.time);
  size_t points_size = m_points.size();

  if (points_size > 1) {
    m_points[points_size - 1].position = { smoothed_point.x, smoothed_point.y };
    // m_points[points_size - 1].pressure = smoothed_point.z;
  }

  vec2 pos = InputManager::pointer.scene.delta;

  // if (points_size > 2) {
  //   vec2 last = m_points[points_size - 1].position;
  //   vec2 last_last = m_points[points_size - 2].position;
  //   float dist = squared_distance(pos, last);

  //   if (dist > 30.0f) {
  //     // console::log("too close");
  //   }

  //   m_points.push_back({ pos, /*InputManager::pointer.pressure*/1.0f });
  // } else {
  m_points.push_back({ pos, /*InputManager::pointer.pressure*/1.0f });
  // }


  // vec2 last_normal;

  Editor::scene.selection.deselect(m_entity->id);
  m_entity = std::make_shared<ElementEntity>(InputManager::pointer.scene.origin);
  // Editor::scene.selection.select(m_entity.get());

  // for (size_t i = 1; i < points_size - 1; i++) {
  //   vec2 direction = m_points[i + 1].position - m_points[i].position;
  //   vec2 normal = orthogonal(direction);
  //   normalize(normal, normal);

  //   m_entity->add_vertex(std::make_shared<VertexEntity>(m_points[i].position, (midpoint(normal, last_normal), m_points[i].pressure)));

  //   last_normal = normal;
  // }

  const int last_commit = 2;

  std::vector<PathPoint> points;
  int last_point = m_curves.size() ? m_curves[m_curves.size() - 1].end_index : 0;

  points.insert(points.end(), m_points.begin() + last_point, m_points.end());

  std::vector<uint> corners = detect_corners(
    points,
    Settings::corners_radius_min,
    Settings::corners_radius_max,
    Settings::corners_angle_threshold,
    Settings::corners_min_distance,
    Settings::corners_samples_max
  );

  size_t corners_len = corners.size();

  if (corners_len < 2) {
    return;
  }

  std::vector<PathBezier> curves;
  std::vector<PathBezier> curves1;
  for (size_t i = 0; i < corners.size() - 1; i++) {
    uint start = corners[i];
    uint end = corners[i + 1];

    fit_path(points, start, end, Settings::max_fit_error / Editor::viewport.zoom(), curves);
  }

  for (int i = 0; i < m_curves.size(); i++) {
    PathBezier& curve = m_curves[i];

    if (!m_entity->vertex_count()) {
      m_entity->add_vertex(std::make_shared<VertexEntity>(curve.p0, curve.p1 - curve.p0, false, curve.pressure.x));
    } else {
      HandleEntity* left = m_entity->last_vertex().left();
      if (left) {
        vec2 l = left->transform()->position().get();
        vec2 r = curve.p1 - curve.p0;

        float a = std::acosf(std::abs(dot(l, r)));
        negate(l, l);

        if (a > MATH_PI / 3 * 2) {
          float ll = length(l);
          float rr = length(r);

          vec2 dir = normalize(midpoint(l / ll, r / rr));

          left->transform()->position().set(-dir * ll);
          m_entity->last_vertex().set_right(dir * rr);
        } else {
          m_entity->last_vertex().set_right(curve.p1 - curve.p0);
        }
      } else {
        m_entity->last_vertex().set_right(curve.p1 - curve.p0);
      }
    }

    m_entity->add_vertex(std::make_shared<VertexEntity>(curve.p3, curve.p2 - curve.p3, true, curve.pressure.y));
  }

  for (int i = 0; i < curves.size(); i++) {
    PathBezier& curve = curves[i];

    if (i < curves.size() - 1 && curve.p0 == curve.p1 && curve.p2 == curve.p3) {
      vec2 prev = points[curve.start_index > 0 ? curve.start_index - 1 : curve.start_index].position;
      vec2 next = points[curve.end_index < points_size - 1 ? curve.end_index + 1 : curve.end_index].position;

      // generate catmull-rom spline


      // vec2 c1 = curve.p0 + (curve.p3 - curve.p0) * 0.33f;
      // vec2 c2 = curve.p3 - (next - curve.p0) * 0.33f;

      // curve.p1 = c1;
      // curve.p2 = c2;

      curve.p1 = (-prev + 6.0f * curve.p0 + curve.p3) / 6.0f;
      curve.p2 = (curve.p0 + 6.0f * curve.p3 - next) / 6.0f;
    }

    if (squared_distance(curve.p1, curve.p0) > squared_distance(points[curve.start_index].position, points[curve.end_index].position) * 3.0f) {
      console::log("bad error");

      // uint diff = curve.end_index - curve.start_index;
      // std::vector<vec2> p1(diff), p2(diff);

      curve.p1 = curve.p0 + (curve.p1 - curve.p0) * 0.5f;
      curve.p2 = curve.p3 + (curve.p2 - curve.p3) * 0.5f;
    }



#if 0
    // Circular regression 
    mat3 A{ 0.0f };
    vec3 B{ 0.0f };
    vec4 C{ 0.0f };

    for (int h = curve.start_index; h <= curve.end_index; h++) {
      vec2 p = points[h].position;

      A[0][0] += p.x * p.x;
      A[0][1] += p.x * p.y;
      A[0][2] += p.x;

      A[1][0] += p.x * p.y;
      A[1][1] += p.y * p.y;
      A[1][2] += p.y;

      A[2][0] += p.x;
      A[2][1] += p.y;
      A[2][2] += 1.0f;

      B[0] += p.x * p.x * p.x + p.x * p.y * p.y;
      B[1] += p.y * p.y * p.y + p.x * p.x * p.y;
      B[2] += p.x * p.x + p.y * p.y;

      C[0] += p.x * p.x;
      C[1] += p.y * p.y;
      C[2] += p.x;
      C[3] += p.y;
    }

    A[0][0] *= 2.0f;
    A[0][1] *= 2.0f;
    A[0][2] *= 2.0f;

    A[1][0] *= 2.0f;
    A[1][1] *= 2.0f;
    A[1][2] *= 2.0f;

    float n = A[2][2];
    mat3 A_inv = inverse(A);
    vec3 AB = A_inv * B;

    float c = (C.x + C.y - 2 * AB.x * C.z - 2 * AB.y * C.w) / n;
    float R = std::sqrtf(AB.x * AB.x + AB.y * AB.y + c);

    float x1 = points[curve.start_index].position.x;
    float y1 = points[curve.start_index].position.y;
    float x4 = points[curve.end_index].position.x;
    float y4 = points[curve.end_index].position.y;
    float xc = AB.x;
    float yc = AB.y;

    float ax = x1 - xc;
    float ay = y1 - yc;
    float bx = x4 - xc;
    float by = y4 - yc;
    float q1 = ax * ax + ay * ay;
    float q2 = q1 + ax * bx + ay * by;
    float k2 = (4 / 3) * (sqrt(2 * q1 * q2) - q2) / (ax * by - ay * bx);

    float x2 = xc + ax - k2 * ay;
    float y2 = yc + ay + k2 * ax;
    float x3 = xc + bx + k2 * by;
    float y3 = yc + by - k2 * bx;

    curve.p1 = vec2(x2, y2);
    curve.p2 = vec2(x3, y3);
#endif
    // for (int h = curve.start_index + 1; h < curve.end_index; h++) {
    //   float t = h / (float)(diff);

    //   float d1 = distance(curve.p0, points[h].position);
    //   float d2 = distance(curve.p1, points[h].position);

    //   float new_t = d1 / (d1 + d2);

    //   float u = (1 - new_t) * (1 - new_t) / (new_t * new_t + (1 - new_t) * (1 - new_t));
    //   float ratio = std::abs((new_t * new_t + (1 - new_t) * (1 - new_t) - 1) / (new_t * new_t + (1 - new_t) * (1 - new_t)));

    //   vec2 C = u * curve.p0 + (1 - u) * curve.p3;
    //   vec2 A = points[h].position + (points[h].position - C) / ratio;

    //   p1[h - curve.start_index - 1] = 0.66f * (A - curve.p0) + curve.p0;
    //   p2[h - curve.start_index - 1] = 0.66f * (A - curve.p3) + curve.p3;
    // }

    // vec2 avg_p1{ 0.0f };
    // vec2 avg_p2{ 0.0f };

    // for (int j = 0; j < p1.size(); j++) {
    //   avg_p1 += p1[j];
    //   avg_p2 += p2[j];
    // }

    // avg_p1 /= p1.size();
    // avg_p2 /= p2.size();

    // curve.p1 = avg_p1;
    // curve.p2 = avg_p2;
    // }

    // if (std::find(corners.begin(), corners.end(), curve.start_index) == corners.end()) {
    //   PathBezier& last_curve = curves[i - 1];

    //   vec2 left_vector = last_curve.p3 - last_curve.p2;
    //   vec2 right_vector = curve.p1 - curve.p0;

    //   if (is_almost_zero(left_vector) || is_almost_zero(right_vector)) {
    //     continue;
    //   }

    //   float left_length = length(left_vector);
    //   float right_length = length(right_vector);

    //   left_vector /= left_length;
    //   right_vector /= right_length;

    //   vec2 new_vector = midpoint(left_vector, right_vector);
    //   normalize(new_vector, new_vector);

    //   curve.p1 = curve.p0 + new_vector * right_length;
    //   last_curve.p3 = last_curve.p2 + new_vector * left_length;
    // }

    curve.start_index += last_point;
    curve.end_index += last_point;
  }

  if (curves.size() > last_commit) {
    m_curves.insert(m_curves.end(), curves.begin(), curves.end() - last_commit);
  }

  for (int i = 0; i < curves.size(); i++) {
    PathBezier& curve = curves[i];

    if (!m_entity->vertex_count()) {
      m_entity->add_vertex(std::make_shared<VertexEntity>(curve.p0, curve.p1 - curve.p0, false, curve.pressure.x));
    } else {
      HandleEntity* left = m_entity->last_vertex().left();
      if (left) {
        vec2 l = left->transform()->position().get();
        vec2 r = curve.p1 - curve.p0;

        float a = std::acosf(std::abs(dot(l, r)));
        negate(l, l);

        if (a > MATH_PI / 3 * 2) {
          float ll = length(l);
          float rr = length(r);

          vec2 dir = normalize(midpoint(l / ll, r / rr));

          left->transform()->position().set(-dir * ll);
          m_entity->last_vertex().set_right(dir * rr);
        } else {
          m_entity->last_vertex().set_right(curve.p1 - curve.p0);
        }
      } else {
        m_entity->last_vertex().set_right(curve.p1 - curve.p0);
      }
    }

    m_entity->add_vertex(std::make_shared<VertexEntity>(curve.p3, curve.p2 - curve.p3, true, curve.pressure.y));
  }
}

void PencilTool::on_pointer_up(bool abort) {
  // Editor::scene.add_entity(m_entity);
  // InputManager::set_tool(ToolType::DirectSelect);

  // m_entity = nullptr;
  // m_points.clear();
  // m_curves.clear();
}

void PencilTool::render_overlays(float zoom) const {
  if (!m_entity) return;

  m_entity->render(zoom);

  // TEMP
  // InstancedGeometry geo;
  // geo.push_circle(m_entity->transform()->position().get(), 7.0f / zoom, vec4{ 0.9f, 0.2f, 0.2f, 1.0f }, 12);

  // for (auto& point : m_points) {
  //   geo.push_instance(point.position);
  // }

  // Renderer::draw(geo);
}

#if 0

#define MAX_ERROR 2
#define FIELD_RADIUS 9
#define NUM_SAMPLE_POINTS 9
#define MAX_ITERATIONS 50
#define MAX_CORNER_ANGLE 80
#define RADIAL_SIMPLIFICATION 1.0f
#define SHOULD_USE_NEW_CORNER_FINDER true
#define SHOULD_USE_MY_FORCE_VECTORS true

PencilTool::PencilTool(): Tool(ToolType::Pencil, CategoryImmediate | CategoryDirect) {}

static vec2 round_vec(vec2 v) {
  return { std::roundf(v.x), std::roundf(v.y) };
}

static PencilTool::Curve create_curve(vec2 pos, vec2 viewport_size) {
  PencilTool::Curve curve;
  curve.segments.push_back({ pos, pos, pos, pos, std::nullopt, 0.0f, PencilTool::UpdateStatus::Undefined, 0 });

  curve.vdmap = std::vector<std::optional<std::vector<std::optional<vec2>>>>((int)viewport_size.x + 1);
  std::fill(curve.vdmap.begin(), curve.vdmap.end(), std::nullopt);

  return curve;
}

void PencilTool::on_pointer_down() {
  m_curve = create_curve(round_vec(InputManager::pointer.client.origin), Editor::viewport.size());
}

static vec2 get_segment_end_tangents(PencilTool::Segment& segment) {
  vec2 endpoint = segment.c3;
  vec2 control_point = segment.c2;

  return 3.0f * (control_point - endpoint);
};

static float radians_to_degrees(float rad) {
  return (rad * 180.0f) / MATH_PI;
};

static bool check_corner(PencilTool::Segment& curve_segment, vec2 point) {
  if (is_almost_equal(curve_segment.c0, curve_segment.c3)) return false;

  vec2 tan;

  if (SHOULD_USE_NEW_CORNER_FINDER) {
    tan = bezier(curve_segment.c0, curve_segment.c1, curve_segment.c2, curve_segment.c3, 0.95f) - curve_segment.c3;
  } else {
    tan = get_segment_end_tangents(curve_segment);
  }

  normalize(tan, tan);

  vec2 new_segment = normalize(point - curve_segment.c3);

  // Dot product == cos(angle) since the magnitude of unit vectors is 1
  float cos = dot(tan, new_segment);
  float angle = radians_to_degrees(std::acosf(cos));

  if (angle < MAX_CORNER_ANGLE) {
    return true;
  }

  return false;
};

static float get_DDA_steps(vec2 diff) {
  return std::max(std::fabsf(diff.x), std::fabsf(diff.y));
}

vec2 get_distance_from_polyline(PencilTool::Curve& curve, vec2 point) {
  int x = (int)point.x;
  int y = (int)point.y;

  console::log("size", curve.vdmap.size());
  if (curve.vdmap[x].has_value()) {
    std::optional<vec2>& first = curve.vdmap[x].value()[y];
    std::optional<vec2>& second = curve.vdmap[x].value()[y + 1];
    vec2 val = first.value_or(vec2{ FIELD_RADIUS });

    if (second.has_value() && std::fabsf(val.x) > std::fabsf(second.value().x)) {
      val.x = second.value().x;
    }
    if (second.has_value() && std::fabsf(val.y) > std::fabsf(second.value().y)) {
      val.y = second.value().y;
    }
    return val;
  } else {
    return vec2{ FIELD_RADIUS };
  }
};

static PencilTool::Segment update_distance_field(PencilTool::Curve& curve, vec2 current_point) {
  PencilTool::Segment& segment = curve.segments[curve.segments.size() - 1];

  vec2 last_point = segment.c3;
  PencilTool::Segment og_controls = segment;

  if (check_corner(segment, current_point)) {
    segment.update_status = PencilTool::UpdateStatus::FailCorner;
    return segment;
  }

  segment.c3 = current_point;
  segment.c2 = segment.c2 + current_point - last_point;

  //
  // Prep the rectangle around our line in which we'll store distance-to-the-line
  // for each pixel in the box.
  vec2 perp_vec = normalize(orthogonal(current_point - last_point));
  vec2 dist = perp_vec * FIELD_RADIUS;
  vec2 a1 = last_point + dist;
  vec2 b1 = current_point + dist;

  vec2 vert_diff = negate(dist * 2.0f);
  float vert_steps = get_DDA_steps(vert_diff);
  vec2 vert_increment = vert_diff / vert_steps;

  vec2 horiz_diff = b1 - a1;
  float horiz_steps = get_DDA_steps(horiz_diff);
  vec2 horiz_increment = horiz_diff / horiz_steps;

  //
  // "Render" the rectangle around the line by interpolating `current_val`
  // up the box (perpindicularly to the line) for each step across the box
  // (parallel to the line). This is a very basic rasterization concept with a
  // scary name: Digital Differential Analyzer (DDA).
  //
  // a1------------------b1  -,
  // |                    |   |-> FIELD_RADIUS
  // |------the line------|  -'
  // |               •----|-------> Sample distance "pixel": { x: 0, y: -1 }
  // a2------------------b2
  vec2 current_horiz_location = a1;
  for (int i = 0; i < horiz_steps; i++) {
    vec2 current_location = current_horiz_location;
    vec2 current_val = dist;
    for (int j = 0; j < vert_steps; j++) {
      int x = std::roundf(current_location.x);
      int y = std::roundf(current_location.y);

      console::log("x", x);
      if (curve.vdmap.size() == 0) {
        console::log("size", curve.vdmap.size());
      }
      if (!curve.vdmap[x].has_value()) curve.vdmap[x] = std::vector<std::optional<vec2>>((int)Editor::viewport.size().y);
      std::fill(curve.vdmap[x].value().begin(), curve.vdmap[x].value().end(), std::nullopt);

      if (!curve.vdmap[x].value()[y].has_value()) {
        curve.vdmap[x].value()[y] = round_vec(current_val);
      } else {
        curve.vdmap[x].value()[y] =
          squared_length(curve.vdmap[x].value()[y].value()) < squared_length(round_vec(current_val))
          ? curve.vdmap[x].value()[y]
          : round_vec(current_val);
      }

      current_val = current_val + vert_increment;
      current_location = current_location + vert_increment;
    }

    current_horiz_location = current_horiz_location + horiz_increment;
  }

  //
  // "Render" a square cap at the endpoint.
  vec2 upper_left_point = current_point - FIELD_RADIUS;
  vec2 bottom_right_point = current_point + FIELD_RADIUS;
  for (int x = upper_left_point.x; x < bottom_right_point.x; x++) {
    for (int y = upper_left_point.y; y < bottom_right_point.y; y++) {
      if (!curve.vdmap[x].has_value()) curve.vdmap[x] = std::vector<std::optional<vec2>>((int)Editor::viewport.size().y);
      vec2 val = vec2{ (float)x, (float)y } - current_point;
      if (!curve.vdmap[x].value()[y].has_value()) {
        curve.vdmap[x].value()[y] = val;
      } else {
        curve.vdmap[x].value()[y] =
          squared_length(curve.vdmap[x].value()[y].value()) < squared_length(val)
          ? curve.vdmap[x].value()[y]
          : val;
      }
    }
  }

  //
  // Trial-and-error over and over to get a curve that fits nicely.
  int steps = 0;
  while (true) {
    vec2 f1{ 0.0f };
    vec2 f2{ 0.0f };

    //
    // Create force vectors by checking the distance (with the vector
    // distance field we've built up above so it's fast) of our
    // iteratively-fitting curve and pushing the control points
    // in the direction that helps most.
    for (int i = 0; i < NUM_SAMPLE_POINTS; i++) {
      float t = (float)i / (float)NUM_SAMPLE_POINTS;

      vec2 point = round_vec(bezier(segment.c0, segment.c1, segment.c2, segment.c3, t));
      console::log("t", point);
      vec2 dist = get_distance_from_polyline(curve, abs(point));
      float d = length(dist);

      float dx = dist.x, dy = dist.y;
      float modifier = 1;

      if (SHOULD_USE_MY_FORCE_VECTORS) {
        if (t < 0.1 || t > 0.9) modifier = 10;
      }

      f1.x += t * std::pow(1 - t, 2) * d * dx * modifier;
      f1.y += t * std::pow(1 - t, 2) * d * dy * modifier;
      f2.x += std::pow(t, 2) * (1 - t) * d * dx * modifier;
      f2.y += std::pow(t, 2) * (1 - t) * d * dy * modifier;
    }

    //
    // Push the force vectors slightly toward the middle to mitigate
    // hooking at the end of the curve.
    if (SHOULD_USE_MY_FORCE_VECTORS) {
      vec2 from_end = midpoint(segment.c0, segment.c3) - segment.c2;
      vec2 from_beginning = midpoint(segment.c0, segment.c3) - segment.c1;
      f1 = f1 - from_beginning * 0.03f;
      f2 = f2 - from_end * 0.03f;
    }

    //
    // Constrain the first control point to adjust itself along the same
    // line as the previous segments second control point so the curve
    // looks continuous.
    if (segment.constrain_to.has_value()) {
      f1 = segment.constrain_to.value() * dot(segment.constrain_to.value(), f1);
    }

    //
    // Apply the force vectors to the control points.
    float alpha = 1;
    segment.c1.x -= (alpha * f1.x * 6) / NUM_SAMPLE_POINTS;
    segment.c1.y -= (alpha * f1.y * 6) / NUM_SAMPLE_POINTS;
    segment.c2.x -= (alpha * f2.x * 6) / NUM_SAMPLE_POINTS;
    segment.c2.y -= (alpha * f2.y * 6) / NUM_SAMPLE_POINTS;

    //
    // Add up the error of the curve (again with our fast distance field).
    float error = 0;
    for (int i = 0; i < NUM_SAMPLE_POINTS; i++) {
      float t = (float)i / (float)NUM_SAMPLE_POINTS;
      vec2 point = bezier(segment.c0, segment.c1, segment.c2, segment.c3, t);
      vec2 dist = get_distance_from_polyline(curve, abs(round_vec(point)));
      error += std::pow(dist.x, 2) + std::pow(dist.y, 2);
    }
    error = error / NUM_SAMPLE_POINTS;
    steps++;

    segment.error = error;
    segment.steps = steps;

    //
    // Do it all again unless the curve is good enough or we've been at it for a bit.
    if (error < MAX_ERROR || steps > MAX_ITERATIONS) break;
  }

  //
  // If we failed, reset the segment back to the way it was.
  if (steps > MAX_ITERATIONS) {
    segment.c0 = og_controls.c0;
    segment.c1 = og_controls.c1;
    segment.c2 = og_controls.c2;
    segment.c3 = og_controls.c3;
    segment.error = og_controls.error;
    segment.constrain_to = og_controls.constrain_to;
    segment.update_status = PencilTool::UpdateStatus::FailMaxed;
    return segment;
  }

  segment.update_status = PencilTool::UpdateStatus::Success;
  return segment;
};


static PencilTool::Segment add_to_curve_impl(PencilTool::Curve& curve, vec2 new_point) {
  PencilTool::Segment& last_segment = curve.segments[curve.segments.size() - 1];
  vec2 last_point = last_segment.c3;

  // We do too much work with too little benefit if points are too close together.
  if (squared_distance(new_point, last_point) < RADIAL_SIMPLIFICATION * RADIAL_SIMPLIFICATION) {
    return last_segment;
  }

  if (
    last_segment.update_status != PencilTool::UpdateStatus::Success &&
    last_segment.update_status != PencilTool::UpdateStatus::Undefined
    ) {
    PencilTool::Segment new_segment{ last_point, last_point, last_point, last_point, std::nullopt, 0.0f, PencilTool::UpdateStatus::Undefined, 0 };
    // Start a new, unconstrained segment for corners.
    if (last_segment.update_status == PencilTool::UpdateStatus::FailMaxed) {
      new_segment.constrain_to = normalize(get_segment_end_tangents(last_segment));
    }

    // Reset the vector distance map because we're fitting a new curve!
    std::fill(curve.vdmap.begin(), curve.vdmap.end(), std::nullopt);
    curve.segments.push_back(new_segment);
  }

  return update_distance_field(curve, new_point);
}

void PencilTool::on_pointer_move() {
  vec2 rounded_coord = round_vec(InputManager::pointer.client.position);
  Segment segment = add_to_curve_impl(m_curve, rounded_coord);

  if (segment.update_status == UpdateStatus::Success) {
    m_curve.segments.push_back(segment);
  }

  m_curve.segments.push_back(add_to_curve_impl(m_curve, rounded_coord));

  m_entity = std::make_shared<ElementEntity>(InputManager::pointer.scene.origin);

  for (auto& curve : m_curve.segments) {

    if (!m_entity->vertex_count()) {
      m_entity->add_vertex(std::make_shared<VertexEntity>(curve.c0, curve.c1 - curve.c0, false, 1.0f));
    } else {
      m_entity->last_vertex().set_right(curve.c1 - curve.c0);
    }

    m_entity->add_vertex(std::make_shared<VertexEntity>(curve.c3, curve.c2 - curve.c3, true, 1.0f));
  }
}

void PencilTool::on_pointer_up(bool abort) {

}

void PencilTool::render_overlays(float zoom) const {
  if (!m_entity) return;

  m_entity->render(zoom);
}

#endif
#if 0

PencilTool::~PencilTool() {
  // delete[] m_vector_dist_map;
}

void PencilTool::on_pointer_down() {
#if 1
  const int width = Editor::viewport.size().x;
  const int height = Editor::viewport.size().y;

  if (width * height != m_vector_dist_map.size()) {
    m_vector_dist_map = std::vector<vec2>(width * height);
  }

  m_curves.clear();

  PathBezier curve;
  curve.p0 = curve.p1 = curve.p2 = curve.p3 = InputManager::pointer.client.position;

  m_curves.push_back(curve);
#else

  m_points.clear();
  m_curves.clear();

  if (m_entity) {
    Editor::scene.selection.deselect(m_entity->id);
  }

  m_entity = std::make_shared<ElementEntity>(InputManager::pointer.scene.position);
  m_points.push_back({ vec2{ 0.0f }, /*InputManager::pointer.pressure*/1.0f });

  WobbleSmoother::reset(
    { InputManager::pointer.type != InputManager::PointerType::Pen, 20.0f, 40.0f, 1.31f, 1.44f },
    vec2{ 0.0f }, /*InputManager::pointer.pressure*/1.0f, InputManager::pointer.time
  );

  m_corners_geo = InstancedGeometry();
  m_corners_geo.push_circle(m_entity->transform()->position().get(), 2.0f / Editor::viewport.zoom(), vec4{ 0.9f, 0.2f, 0.2f, 1.0f }, 12);
#endif
  // Editor::scene.add_entity(m_entity);
}

static bool test_corner(vec2 position, PathBezier& curve) {
  vec2 dir1 = position - curve.p3;
  vec2 dir2 = bezier_derivative(curve.p0, curve.p1, curve.p2, curve.p3, 1.0f);

  if (is_almost_zero(dir2)) {
    dir2 = bezier_second_derivative(curve.p0, curve.p1, curve.p2, curve.p3, 1.0f);
  }

  normalize(dir1, dir1);
  normalize(dir2, dir2);

  if (std::acos(dot(dir1, dir2)) > MATH_PI / 3.0f) return true;

  return false;
}

static void render_line_cell(vec2 prev, vec2 position, float radius, std::vector<vec2>& field) {
  vec2 dir = position - prev;
}

static int add_to_curve_impl(vec2 position, PathBezier& curve) {
  const last_segment = getLastSegment(curve);
  const last_point = getLastPoint(last_segment);

  // We do too much work with too little benefit if points are too close together.
  if (getMagnitude(new_point, getLastPoint(last_segment)) < RADIAL_SIMPLIFICATION)
    return last_segment;

  if (
    UpdateStatus.SUCCESS != last_segment.update_status &&
    last_segment.update_status != = undefined
    ) {
    let new_segment;
    // Start a new, unconstrained segment for corners.
    if (UpdateStatus.FAIL_CORNER == last_segment.update_status) {
      new_segment = initCurveSegment(last_point.x, last_point.y);
      // We had to give up, so continue the curve, but with a new segment.
    } else if (UpdateStatus.FAIL_MAXED == last_segment.update_status) {
      new_segment = initCurveSegment(last_point.x, last_point.y);
      new_segment.constrain_to = getUnitVector(getCurveEndTangent(last_segment));
    }

    // Reset the vector distance map because we're fitting a new curve!
    curve.vdmap = [];
    curve.segments.push(new_segment);
  }

  return updateDistanceField(curve, new_point);
}

static int add_to_curve(vec2 position, PathBezier& curve) {
  vec2 rounded_coord = { std::round(position.x), std::round(position.y) };

  // if (test_corner(position, curve)) return 2;

  // vec2 prev = curve.p3;

  // curve.p3 = position;
  // curve.p2 = curve.p2 + position - prev;

  // render_line_cell(prev, position, 100.0f, m_vector_dist_map);
  // render_point_cell(prev, position, 100.0f, m_vector_dist_map);
}

#if 0
void PencilTool::on_pointer_move() {
  if (!m_entity) return;

  vec3 smoothed_point = WobbleSmoother::update(InputManager::pointer.scene.delta, /*InputManager::pointer.pressure*/1.0f, InputManager::pointer.time);
  size_t points_size = m_points.size();

  if (points_size > 1) {
    m_points[points_size - 1].position = { smoothed_point.x, smoothed_point.y };
    m_points[points_size - 1].pressure = smoothed_point.z;
  }

  m_points.push_back({ InputManager::pointer.scene.delta, /*InputManager::pointer.pressure*/1.0f });

  m_entity = std::make_shared<ElementEntity>(InputManager::pointer.scene.origin);

  std::vector<uint> corners = detect_corners(
    m_points,
    Settings::corners_radius_min,
    Settings::corners_radius_max,
    Settings::corners_angle_threshold,
    Settings::corners_min_distance,
    Settings::corners_samples_max
  );

  std::vector<PathBezier> curves;
  for (size_t i = 0; i < corners.size() - 1; i++) {
    uint start = corners[i];
    uint end = corners[i + 1];

    if (m_entity->vertex_count()) {
      m_entity->last_vertex().set_right((m_points[start + 1].position - m_points[start].position) * 0.33f);
    } else {
      m_entity->add_vertex(std::make_shared<VertexEntity>(
        m_points[start].position,
        (m_points[start + 1].position - m_points[start].position) * 0.33f,
        false,
        m_points[start].pressure
      ));
    }

    for (int j = start + 1; j < end; j++) {
      m_entity->add_vertex(std::make_shared<VertexEntity>(
        m_points[j].position,
        -(m_points[j + 1].position - m_points[j - 1].position) * 0.33f,
        (m_points[j + 1].position - m_points[j].position) * 0.33f,
        m_points[j].pressure
      ));
    }

    m_entity->add_vertex(std::make_shared<VertexEntity>(
      m_points[end].position,
      (m_points[end - 1].position - m_points[end].position) * 0.33f,
      true,
      m_points[end].pressure
    ));
  }
}
#else 
void PencilTool::on_pointer_move() {
#if 1
  const int update_result = add_to_curve(InputManager::pointer.client.position, m_curves[m_curves.size() - 1]);
#else
  if (!m_entity) return;

  vec3 smoothed_point = WobbleSmoother::update(InputManager::pointer.scene.delta, /*InputManager::pointer.pressure*/1.0f, InputManager::pointer.time);
  size_t points_size = m_points.size();

  if (points_size > 1) {
    m_points[points_size - 1].position = { smoothed_point.x, smoothed_point.y };
    m_points[points_size - 1].pressure = smoothed_point.z;
  }

  m_points.push_back({ InputManager::pointer.scene.delta, /*InputManager::pointer.pressure*/1.0f });

  // vec2 last_normal;

  Editor::scene.selection.deselect(m_entity->id);
  m_entity = std::make_shared<ElementEntity>(InputManager::pointer.scene.origin);
  Editor::scene.selection.select(m_entity.get());

  // for (size_t i = 1; i < points_size - 1; i++) {
  //   vec2 direction = m_points[i + 1].position - m_points[i].position;
  //   vec2 normal = orthogonal(direction);
  //   normalize(normal, normal);

  //   m_entity->add_vertex(std::make_shared<VertexEntity>(m_points[i].position, (midpoint(normal, last_normal), m_points[i].pressure)));

  //   last_normal = normal;
  // }

  const int last_commit = 2;

  std::vector<PathPoint> points;
  int last_point = m_curves.size() ? m_curves[m_curves.size() - 1].end_index : 0;

  points.insert(points.end(), m_points.begin() + last_point, m_points.end());

  std::vector<uint> corners = detect_corners(
    points,
    Settings::corners_radius_min,
    Settings::corners_radius_max,
    Settings::corners_angle_threshold,
    Settings::corners_min_distance,
    Settings::corners_samples_max
  );

  size_t corners_len = corners.size();

  if (corners_len < 2) {
    return;
  }

  std::vector<PathBezier> curves;
  for (size_t i = 0; i < corners.size() - 1; i++) {
    uint start = corners[i];
    uint end = corners[i + 1];

    refit_path(points, start, end, Settings::max_fit_error / Editor::viewport.zoom(), curves);
  }

  for (int i = 0; i < m_curves.size(); i++) {
    PathBezier& curve = m_curves[i];

    if (!m_entity->vertex_count()) {
      m_entity->add_vertex(std::make_shared<VertexEntity>(curve.p0, curve.p1 - curve.p0, false, curve.pressure.x));
    } else {
      m_entity->last_vertex().set_right(curve.p1 - curve.p0);
    }

    m_entity->add_vertex(std::make_shared<VertexEntity>(curve.p3, curve.p2 - curve.p3, true, curve.pressure.y));
  }

  for (int i = 0; i < curves.size(); i++) {
    PathBezier& curve = curves[i];

    // if (std::find(corners.begin(), corners.end(), curve.start_index) == corners.end()) {
    //   PathBezier& last_curve = curves[i - 1];

    //   vec2 left_vector = last_curve.p3 - last_curve.p2;
    //   vec2 right_vector = curve.p1 - curve.p0;

    //   if (is_almost_zero(left_vector) || is_almost_zero(right_vector)) {
    //     continue;
    //   }

    //   float left_length = length(left_vector);
    //   float right_length = length(right_vector);

    //   left_vector /= left_length;
    //   right_vector /= right_length;

    //   vec2 new_vector = midpoint(left_vector, right_vector);
    //   normalize(new_vector, new_vector);

    //   curve.p1 = curve.p0 + new_vector * right_length;
    //   last_curve.p3 = last_curve.p2 + new_vector * left_length;
    // }

    curve.start_index += last_point;
    curve.end_index += last_point;
  }

  if (curves.size() > last_commit) {
    for (int i = 0; i < curves.size() - last_commit; i++) {
      m_corners_geo.push_instance(curves[i].p3);
    }

    m_curves.insert(m_curves.end(), curves.begin(), curves.end() - last_commit);
  }

  for (int i = 0; i < curves.size(); i++) {
    PathBezier& curve = curves[i];

    if (!m_entity->vertex_count()) {
      m_entity->add_vertex(std::make_shared<VertexEntity>(curve.p0, curve.p1 - curve.p0, false, curve.pressure.x));
    } else {
      m_entity->last_vertex().set_right(curve.p1 - curve.p0);
    }

    m_entity->add_vertex(std::make_shared<VertexEntity>(curve.p3, curve.p2 - curve.p3, true, curve.pressure.y));
  }
#endif
}
#endif

void PencilTool::on_pointer_up(bool abort) {
  // Editor::scene.add_entity(m_entity);
  // InputManager::set_tool(ToolType::DirectSelect);

  // m_entity = nullptr;
  // m_points.clear();
  // m_curves.clear();
}

void PencilTool::render_overlays(float zoom) const {
  if (!m_entity) return;

  m_entity->render(zoom);

  // TEMP
  InstancedGeometry geo;
  geo.push_circle(m_entity->transform()->position().get(), 2.0f / zoom, vec4{ 0.9f, 0.2f, 0.2f, 1.0f }, 12);

  for (auto& point : m_points) {
    geo.push_instance(point.position);
  }

  Renderer::draw(geo);
}

#endif

#endif