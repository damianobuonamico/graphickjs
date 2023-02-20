#pragma once

#ifdef GK_CONF_DEBUG

#include "../math/vec2.h"
#include "../math/vec4.h"
#include "../math/Box.h"

#include <vector>

#define DEBUGGER_INIT() Debugger::init()
#define DEBUGGER_SHUTDOWN() Debugger::shutdown()
#define DEBUGGER_UPDATE() Debugger::update()

class BezierEntity;
struct Geometry;

class Debugger {
public:
  Debugger(const Debugger&) = delete;
  Debugger(Debugger&&) = delete;

  inline static Debugger* get() { return s_instance; };

  static void init();
  static void shutdown();
  static void update();
private:
  Debugger() = default;

  void render();
  void push_frame(vec2& size, bool align_right = true);

  void render_bezier_outline(const BezierEntity* entity);
  void render_bezier_hodograph(const BezierEntity* entity);
  void render_bezier_curvature(const BezierEntity* entity);
  void render_bezier_geometry(const BezierEntity* entity);
  void render_bezier_triangulation(const BezierEntity* entity);

  void draw_polar_line(vec2& origin, float angle, const Box& boundary, const vec4& color, Geometry& geo);
private:
  float m_frame_offset_left = 0.0f;
  float m_frame_offset_right = 0.0f;
  float m_frame_width = 0.0f;
  vec2 m_cursor{ 0.0f };

  float m_padding = 10.0f;

  std::vector<float> m_t_values;
private:
  static Debugger* s_instance;
};

#else

#define DEBUGGER_INIT()
#define DEBUGGER_UPDATE()
#define DEBUGGER_SHUTDOWN()

#endif