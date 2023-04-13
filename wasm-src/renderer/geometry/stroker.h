#pragma once

#include "../renderer.h"
#include "geometry.h"

enum class JoinType {
  Miter = 0,
  Round,
  Bevel
};

enum class CapType {
  Butt = 0,
  Round,
  Square
};

struct JoinParams {
  vec2 direction;
  vec2 normal;
  uint32_t index;
};

struct TessellationParams {
  vec2 offset;
  RenderingOptions rendering_options;

  float width;
  vec4 color;

  JoinType join;
  CapType cap;
  float miter_limit;

  bool start_join;
  bool end_join;
  bool start_cap;
  bool end_cap;
  bool is_first_segment;

  JoinParams start_join_params;
  JoinParams end_join_params;
};

void tessellate_join(const TessellationParams& params, const vec2& point, const vec2& direction, const vec2& normal, float width, const uint32_t* override_end_index, Geometry& geo);

void tessellate_cap(const TessellationParams& params, const vec2& point, const vec2& normal, bool is_end_cap, float width, Geometry& geo);
