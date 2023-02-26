/*
 * Copyright (c) 2016, Blender Foundation.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "stroker.h"
#include "../../math/vector.h"

#include <vector>

typedef unsigned int uint;

inline extern float min_radius = 0.5f;
inline extern float max_radius = 2.45f;
inline extern uint max_iterations = 5;
inline extern float min_angle = MATH_PI / 8.0f;
inline extern bool simplify_first = true;
inline extern float simplification_tolerance = 0.05f;
inline extern float max_fit_error = 0.95f;

static float cos(const vec2& v0, const vec2& v1, const vec2& v2) {
  vec2 dvec0 = normalize(v0 - v1);
  vec2 dvec1 = normalize(v1 - v2);
  return clamp(dot(dvec0, dvec1), -1.0, 1.0);
}

static float angle(const vec2& v0, const vec2& v1, const vec2& v2) {
  return acos(cos(v0, v1, v2));
}

static bool isect_line_sphere(
  const vec2& l1,
  const vec2& l2,
  const vec2& sp,
  const float r,
  vec2& r_p1
) {

  vec2 ldir = l2 - l1;
  vec2 tvec = l1 - sp;

  const float a = squared_length(ldir);
  const float b = 2.0f * dot(ldir, tvec);
  const float c = squared_length(sp) + squared_length(l1) - (2.0f * dot(sp, l1)) - r * r;

  const float i = b * b - 4.0f * a * c;

  if ((i < 0.0f) || (a == 0.0f)) {
    return false;
  } else if (i == 0.0f) {
    /* one intersection */
    const float mu = -b / (2.0f * a);
    zero(r_p1);
    r_p1 += ldir * mu + l1;
    return true;
  } else if (i > 0.0f) {
    /* # avoid calc twice */
    const float i_sqrt = sqrt(i);

    /* Note: when l1 is inside the sphere and l2 is outside.
     * the first intersection point will always be between the pair. */

     /* first intersection */
    float mu = (-b + i_sqrt) / (2.0f * a);
    zero(r_p1);
    r_p1 += ldir * mu + l1;
    return true;
  } else {
    return false;
  }
}


static bool point_corner_measure(
  const std::vector<FreehandPathPoint>& points,
  const uint points_len,
  const uint i,
  const uint i_prev_init,
  const uint i_next_init,
  const float radius,
  const uint samples_max,
  vec2& r_p_prev, uint* r_i_prev_next,
  vec2& r_p_next, uint* r_i_next_prev)
{
  const vec2 p = points[i].position;

  uint i_prev = i_prev_init;
  uint i_prev_next = i_prev + 1;

  uint sample = 0;

  while (true) {
    if ((i_prev == -1) || (sample++ > samples_max)) {
      return false;
    } else if (squared_distance(p, points[i_prev].position) < radius) {
      i_prev -= 1;
    } else {
      break;
    }
  }

  uint i_next = i_next_init;
  uint i_next_prev = i_next - 1;

  sample = 0;

  while (true) {
    if ((i_next == points_len) || (sample++ > samples_max)) {
      return false;
    } else if (squared_distance(p, points[i_next].position) < radius) {
      i_next += 1;
    } else {
      break;
    }
  }

  /* find points on the sphere */
  if (!isect_line_sphere(
    points[i_prev].position, points[i_prev_next].position, p, radius, r_p_prev
  )) {
    return false;
  }

  if (!isect_line_sphere(
    points[i_next].position, points[i_next_prev].position, p, radius, r_p_next
  )) {
    return false;
  }

  *r_i_prev_next = i_prev_next;
  *r_i_next_prev = i_next_prev;

  return true;
}

static float point_corner_angle(
  const std::vector<FreehandPathPoint>& points,
  const uint points_len,
  const uint i,
  const float radius_mid,
  const float radius_max,
  const float angle_threshold,
  const float angle_threshold_cos,
  const uint samples_max
) {
  assert(angle_threshold_cos == cos(angle_threshold));

  if (i == 0 || i == points_len - 1) {
    return 0.0;
  }

  const vec2& p = points[i].position;

  /* initial test */
  if (cos(points[i - 1].position, p, points[i + 1].position) > angle_threshold_cos) {
    return 0.0f;
  }

  vec2 p_mid_prev;
  vec2 p_mid_next;

  uint i_mid_prev_next, i_mid_next_prev;
  if (point_corner_measure(
    points, points_len,
    i, i - 1, i + 1,
    radius_mid,
    samples_max,
    p_mid_prev, &i_mid_prev_next,
    p_mid_next, &i_mid_next_prev
  )) {
    const float angle_mid_cos = cos(p_mid_prev, p, p_mid_next);

    /* compare as cos and flip direction */
    if (angle_mid_cos < angle_threshold_cos) {
      vec2 p_max_prev;
      vec2 p_max_next;

      uint i_max_prev_next, i_max_next_prev;
      if (point_corner_measure(
        points, points_len,
        i, i - 1, i + 1,
        radius_max,
        samples_max,
        p_max_prev, &i_max_prev_next,
        p_max_next, &i_max_next_prev))
      {
        const float angle_mid = acos(angle_mid_cos);
        const float angle_max = angle(p_max_prev, p, p_max_next) / 2.0f;
        const float angle_diff = angle_mid - angle_max;

        if (angle_diff > angle_threshold) {
          return angle_diff;
        }
      }
    }
  }

  return 0.0f;
}

static std::vector<uint> detect_corners(
  const std::vector<FreehandPathPoint>& points,
  const float radius_min,
  const float radius_max,
  const uint samples_max,
  const float angle_threshold
) {
  const float angle_threshold_cos = cos(angle_threshold);
  const uint points_len = (uint)points.size();
  uint corners_len = 0;

  /* Use the difference in angle between the mid-max radii
   * to detect the difference between a corner and a sharp turn. */
  const float radius_mid = (radius_min + radius_max) / 2.0f;

  /* we could ignore first/last- but simple to keep aligned with the point array */
  std::vector<float> points_angle(points_len);
  points_angle[0] = 0.0f;

  std::vector<uint> corners{};

  for (uint i = 0; i < points_len; i++) {
    points_angle[i] = point_corner_angle(
      points, points_len, i,
      radius_mid, radius_max,
      angle_threshold, angle_threshold_cos,
      samples_max
    );

    if (points_angle[i] != 0.0f) {
      corners_len++;
    }
  }

  if (corners_len == 0) {
    corners.insert(corners.begin(), { 0, points_len - 1 });
    return corners;
  }

  /* Clean angle limits!
   *
   * How this works:
   * - Find contiguous 'corners' (where the distance is less or equal to the error threshold).
   * - Keep track of the corner with the highest angle
   * - Clear every other angle (so they're ignored when setting corners). */
  {
    const float radius_min_sq = radius_min * radius_min;
    uint i_span_start = 0;

    while (i_span_start < points_len) {
      uint i_span_end = i_span_start;

      if (points_angle[i_span_start] != 0.0f) {
        uint i_next = i_span_start + 1;
        uint i_best = i_span_start;

        while (i_next < points_len) {
          if ((points_angle[i_next] == 0.0f) ||
            (squared_distance(
              points[i_next - 1].position,
              points[i_next].position) > radius_min_sq))
          {
            break;
          } else {
            if (points_angle[i_best] < points_angle[i_next]) {
              i_best = i_next;
            }

            i_span_end = i_next;
            i_next += 1;
          }
        }

        if (i_span_start != i_span_end) {
          uint i = i_span_start;

          while (i <= i_span_end) {
            if (i != i_best) {
              /* we could use some other error code */
              assert(points_angle[i] != 0.0f);
              points_angle[i] = 0.0f;
              corners_len--;
            }

            i += 1;
          }
        }
      }

      i_span_start = i_span_end + 1;
    }
    /* End angle limit cleaning! */

    corners_len += 2;  /* first and last */
    corners.push_back(0);
    uint i_corner = 1;

    for (uint i = 0; i < points_len; i++) {
      if (points_angle[i] != 0.0f) {
        corners.push_back(i);
        i_corner++;
      }
    }

    corners.push_back(points_len - 1);
    i_corner++;
    assert(i_corner == corners_len);

    return corners;
  }
}
