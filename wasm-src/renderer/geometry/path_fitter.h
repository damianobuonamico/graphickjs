/*
 * Copyright (c) 2016, DWANGO Co., Ltd.
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
#include "../../utils/console.h"

#include <vector>

 /** Take curvature into account when calculating the least square solution isn't usable. */
#define USE_CIRCULAR_FALLBACK 0

/**
 * Use the maximum distance of any points from the direct line between 2 points
 * to calculate how long the handles need to be.
 * Can do a 'perfect' reversal of subdivision when for curve has symmetrical handles and doesn't change direction
 * (as with an 'S' shape).
 */
#define USE_OFFSET_FALLBACK 0

 /** Avoid re-calculating lengths multiple times. */
#define USE_LENGTH_CACHE 0

/**
 * Store the indices in the cubic data so we can return the original indices,
 * useful when the caller has data associated with the curve.
 */
#define USE_ORIG_INDEX_DATA 0

typedef unsigned int uint;

enum {
  CURVE_FIT_CALC_HIGH_QUALIY = (1 << 0),
  CURVE_FIT_CALC_CYCLIC = (1 << 1),
};

struct Cubic {
  vec2 p0, p1, p2, p3;
};

static float points_calc_coord_length(
  const std::vector<FreehandPathPoint>& points,
  const uint points_offset,
  const uint points_offset_len,
  std::vector<float>& u
) {
  const FreehandPathPoint* pt_prev = &points[points_offset];
  const FreehandPathPoint* pt = &points[points_offset + 1];

  u[0] = 0.0f;

  for (uint i = 1, i_prev = 0; i < points_offset_len; i++) {
    float len;

    len = length(pt->position - pt_prev->position);

    u[i] = u[i_prev] + len;
    i_prev = i;
    pt_prev = pt;
    pt += 1;
  }

  assert(!is_almost_zero(u[points_offset_len - 1]));

  const float w = u[points_offset_len - 1];
  for (uint i = 1; i < points_offset_len; i++) {
    u[i] /= w;
  }

  return w;
}

/**
 * Bezier multipliers
 */

static float B1(float u) {
  float tmp = 1.0f - u;
  return 3.0f * u * tmp * tmp;
}

static float B2(float u) {
  return 3.0f * u * u * (1.0f - u);
}

static float B0plusB1(float u) {
  float tmp = 1.0f - u;
  return tmp * tmp * (1.0f + 2.0f * u);
}

static float B2plusB3(float u) {
  return u * u * (3.0f - 2.0f * u);
}

static vec2 points_calc_center_weighted(
  const std::vector<FreehandPathPoint>& points,
  const uint points_offset,
  const uint points_offset_len
) {
  /*
   * Calculate a center that compensates for point spacing.
   */

  const FreehandPathPoint* pt_prev = &points[points_offset + points_offset_len - 2];
  const FreehandPathPoint* pt_curr = pt_prev + 1;
  const FreehandPathPoint* pt_next = &points[points_offset];

  float w_prev = distance(pt_prev->position, pt_curr->position);

  vec2 center{ 0.0f };
  float w_tot = 0.0;

  for (uint i_next = 0; i_next < points_offset_len; i_next++) {
    const float w_next = distance(pt_curr->position, pt_next->position);
    const float w = w_prev + w_next;
    w_tot += w;

    center += pt_curr->position * w;

    w_prev = w_next;

    pt_prev = pt_curr;
    pt_curr = pt_next;
    pt_next++;
  }

  if (w_tot != 0.0f) {
    center *= 1.0f / w_tot;
  }

  return center;
}

/**
 * Use least-squares method to find Bezier control points for region.
 */
static void cubic_from_points(
  const std::vector<FreehandPathPoint>& points,
  const uint points_offset,
  const uint points_offset_len,
  const std::vector<float> u_prime,
  const vec2& tan_l,
  const vec2& tan_r,
  Cubic& r_cubic
) {

  const vec2 p0 = points[points_offset].position;
  const vec2 p3 = points[points_offset + points_offset_len - 1].position;

  /* Point Pairs. */
  float alpha_l, alpha_r;
  vec2 a[2];

  {
    float x[2] = { 0.0f }, c[2][2] = { {0.f} };
    const FreehandPathPoint* pt = &points[points_offset];

    for (uint i = 0; i < points_offset_len; i++, pt++) {
      a[0] = tan_l * B1(u_prime[i]);
      a[1] = tan_r * B2(u_prime[i]);

      const float b0_plus_b1 = B0plusB1(u_prime[i]);
      const float b2_plus_b3 = B2plusB3(u_prime[i]);

      /* Inline dot product. */
      for (uint j = 0; j < 2; j++) {
        const float tmp = (pt->position[j] - (p0[j] * b0_plus_b1)) + (p3[j] * b2_plus_b3);

        x[0] += a[0][j] * tmp;
        x[1] += a[1][j] * tmp;

        c[0][0] += a[0][j] * a[0][j];
        c[0][1] += a[0][j] * a[1][j];
        c[1][1] += a[1][j] * a[1][j];
      }

      c[1][0] = c[0][1];
    }

    float det_C0_C1 = c[0][0] * c[1][1] - c[0][1] * c[1][0];
    float det_C_0X = x[1] * c[0][0] - x[0] * c[0][1];
    float det_X_C1 = x[0] * c[1][1] - x[1] * c[0][1];

    if (is_almost_zero(det_C0_C1)) {
      det_C0_C1 = c[0][0] * c[1][1] * 10e-12f;
    }

    /* May still divide-by-zero, check below will catch NAN values. */
    alpha_l = det_X_C1 / det_C0_C1;
    alpha_r = det_C_0X / det_C0_C1;
  }

  /*
   * The problem that the stupid values for alpha dare not put
   * only when we realize that the sign and wrong,
   * but even if the values are too high.
   * But how do you evaluate it?
   *
   * Meanwhile, we should ensure that these values are sometimes
   * so only problems absurd of approximation and not for bugs in the code.
   */

  bool use_clamp = true;

  /* Flip check to catch NAN values. */
  if (!(alpha_l >= 0.0f) || !(alpha_r >= 0.0f)) {
    alpha_l = alpha_r = length(p0 - p3) / 3.0f;

    /* Skip clamping when we're using default handles. */
    use_clamp = false;
  }

  r_cubic.p0 = p0;
  r_cubic.p3 = p3;

  vec2& p1 = r_cubic.p1 = p0 - (tan_l * alpha_l);
  vec2& p2 = r_cubic.p2 = p3 + (tan_r * alpha_r);

  /* ------------------------------------
   * Clamping (we could make it optional)
   */
  if (use_clamp) {
    vec2 center = points_calc_center_weighted(points, points_offset, points_offset_len);

    const float clamp_scale = 3.0;  /* Clamp to 3x. */
    float dist_sq_max = 0.0;

    {
      const FreehandPathPoint* pt = &points[points_offset];
      for (uint i = 0; i < points_offset_len; i++, pt++) {
        /* Do inline. */
        float dist_sq_test = 0.0f;
        for (uint j = 0; j < 2; j++) {
          dist_sq_test += std::powf((pt->position[j] - center[j]) * clamp_scale, 2);
        }

        dist_sq_max = std::max(dist_sq_max, dist_sq_test);
      }
    }

    float p1_dist_sq = squared_distance(center, p1);
    float p2_dist_sq = squared_distance(center, p2);

    if (p1_dist_sq > dist_sq_max || p2_dist_sq > dist_sq_max) {
      alpha_l = alpha_r = distance(p0, p3) / 3.0f;

      for (uint j = 0; j < 2; j++) {
        p1[j] = p0[j] - (tan_l[j] * alpha_l);
        p2[j] = p3[j] + (tan_r[j] * alpha_r);
      }

      p1_dist_sq = squared_distance(center, p1);
      p2_dist_sq = squared_distance(center, p2);
    }

    /* Clamp within the 3x radius. */
    if (p1_dist_sq > dist_sq_max) {
      p1 -= center;
      p1 *= sqrt(dist_sq_max) / sqrt(p1_dist_sq);
      p1 += center;
    }
    if (p2_dist_sq > dist_sq_max) {
      p2 -= center;
      p2 *= sqrt(dist_sq_max) / sqrt(p2_dist_sq);
      p2 += center;
    }
  }
  /* End clamping. */
}

/**
 * Cubic Evaluation
 */

static vec2 cubic_calc_point(const Cubic& cubic, const float t) {
  const float s = 1.0f - t;
  vec2 vec;

  for (uint j = 0; j < 2; j++) {
    const float p01 = (cubic.p0[j] * s) + (cubic.p1[j] * t);
    const float p12 = (cubic.p1[j] * s) + (cubic.p2[j] * t);
    const float p23 = (cubic.p2[j] * s) + (cubic.p3[j] * t);

    vec[j] = ((((p01 * s) + (p12 * t))) * s) + ((((p12 * s) + (p23 * t))) * t);
  }

  return vec;
}


static vec2 cubic_calc_speed(const Cubic& cubic, const float t) {
  const float s = 1.0f - t;
  vec2 vec;

  for (uint j = 0; j < 2; j++) {
    vec[j] = 3.0f * ((cubic.p1[j] - cubic.p0[j]) * s * s + 2.0f *
      (cubic.p2[j] - cubic.p0[j]) * s * t +
      (cubic.p3[j] - cubic.p2[j]) * t * t);
  }

  return vec;
}

static vec2 cubic_calc_acceleration(const Cubic& cubic, const float t) {
  const float s = 1.0f - t;
  vec2 vec;

  for (uint j = 0; j < 2; j++) {
    vec[j] = 6.0f * ((cubic.p2[j] - 2.0f * cubic.p1[j] + cubic.p0[j]) * s +
      (cubic.p3[j] - 2.0f * cubic.p2[j] + cubic.p1[j]) * t);
  }

  return vec;
}

/**
 * Returns a 'measure' of the maximum distance (squared) of the points specified
 * by points_offset from the corresponding cubic(u[]) points.
 */
static float cubic_calc_error(
  const Cubic& cubic,
  const std::vector<FreehandPathPoint>& points,
  const uint points_offset,
  const uint points_offset_len,
  const std::vector<float>& u,
  uint* r_error_index
) {
  float error_max_sq = 0.0f;
  uint error_index = 0;

  const FreehandPathPoint* pt_real = &points[points_offset + 1];
  vec2 pt_eval;

  for (uint i = 1; i < points_offset_len - 1; i++, pt_real++) {
    pt_eval = cubic_calc_point(cubic, u[i]);

    const float err_sq = squared_distance(pt_real->position, pt_eval);
    if (err_sq >= error_max_sq) {
      error_max_sq = err_sq;
      error_index = i;
    }
  }

  *r_error_index = error_index;
  return error_max_sq;
}

/**
 * Use Newton-Raphson iteration to find better root.
 *
 * \param cubic: Current fitted curve.
 * \param p: Point to test against.
 * \param u: Parameter value for \a p.
 *
 * \note Return value may be `nan` caller must check for this.
 */
static float cubic_find_root(
  const Cubic& cubic,
  const vec2 p,
  const float u) {
  /* Newton-Raphson Method. */
  /* All vectors. */
  vec2 q0_u = cubic_calc_point(cubic, u);
  vec2 q1_u = cubic_calc_speed(cubic, u);
  vec2 q2_u = cubic_calc_acceleration(cubic, u);

  /* May divide-by-zero, caller must check for that case. */
  q0_u -= p;

  return u - dot(q0_u, q1_u) / (squared_length(q1_u) + dot(q0_u, q2_u));
}

/**
 * Given set of points and their parameterization, try to find a better parameterization.
 */
static bool cubic_reparameterize(
  const Cubic& cubic,
  const std::vector<FreehandPathPoint>& points,
  const uint points_offset,
  const uint points_offset_len,
  const std::vector<float>& u,
  std::vector<float>& r_u_prime
) {
  /*
   * Recalculate the values of u[] based on the Newton Raphson method
   */

  const FreehandPathPoint* pt = &points[points_offset];
  for (uint i = 0; i < points_offset_len; i++, pt++) {
    r_u_prime[i] = cubic_find_root(cubic, pt->position, u[i]);
    if (!isfinite(r_u_prime[i])) {
      return false;
    }
  }

  std::sort(r_u_prime.begin(), r_u_prime.end());

  if ((r_u_prime[0] < 0.0) ||
    (r_u_prime[points_offset_len - 1] > 1.0f))
  {
    return false;
  }

  assert(r_u_prime[0] >= 0.0f);
  assert(r_u_prime[points_offset_len - 1] <= 1.0f);
  return true;
}

static bool fit_cubic_to_points(
  const std::vector<FreehandPathPoint>& points,
  const uint points_offset,
  const uint points_offset_len,
  const vec2& tan_l,
  const vec2& tan_r,
  const float error_threshold_sq,
  Cubic& r_cubic,
  float* r_error_max_sq,
  uint* r_split_index
) {
  const uint iteration_max = 4;

  if (points_offset_len == 2) {
    r_cubic.p0 = points[points_offset + 0].position;
    r_cubic.p3 = points[points_offset + 1].position;

    const float dist = distance(r_cubic.p0, r_cubic.p3) / 3.0f;

    r_cubic.p1 = r_cubic.p0 - tan_l * dist;
    r_cubic.p2 = r_cubic.p3 - tan_r * dist;

    return true;
  }

  std::vector<float> u(points_offset_len);

  points_calc_coord_length(points, points_offset, points_offset_len, u);

  float error_max_sq;
  uint split_index;

  /* Parameterize points, and attempt to fit curve. */
  cubic_from_points(
    points, points_offset, points_offset_len,
    u, tan_l, tan_r, r_cubic
  );

  /* Find max deviation of points to fitted curve. */
  error_max_sq = cubic_calc_error(
    r_cubic, points, points_offset, points_offset_len, u,
    &split_index
  );

  Cubic cubic_test{};

  *r_error_max_sq = error_max_sq;
  *r_split_index = split_index;

  if (!(error_max_sq < error_threshold_sq)) {
    cubic_test = r_cubic;

    /* If error not too large, try some re-parameterization and iteration. */
    std::vector<float> u_prime(points_offset_len);
    for (uint iter = 0; iter < iteration_max; iter++) {
      if (!cubic_reparameterize(
        cubic_test, points, points_offset, points_offset_len, u, u_prime
      )) {
        break;
      }

      cubic_from_points(
        points, points_offset, points_offset_len,
        u_prime, tan_l, tan_r, cubic_test
      );

      const float error_max_sq_test = cubic_calc_error(
        cubic_test, points, points_offset, points_offset_len, u_prime,
        &split_index
      );

      if (error_max_sq > error_max_sq_test) {
        error_max_sq = error_max_sq_test;
        r_cubic = cubic_test;
        *r_error_max_sq = error_max_sq;
        *r_split_index = split_index;
      }

      if (!(error_max_sq < error_threshold_sq)) {
        /* Continue. */
      } else {
        assert((error_max_sq < error_threshold_sq));
        return true;
      }

      u.swap(u_prime);
    }

    return false;
  }

  return true;
}

static void fit_cubic_to_points_recursive(
  const std::vector<FreehandPathPoint>& points,
  const uint points_offset,
  const uint points_offset_len,
  const vec2& tan_l,
  const vec2& tan_r,
  const float error_threshold_sq,
  const uint calc_flag,
  std::vector<Cubic>& curves
) {
  Cubic cubic;
  uint split_index;
  float error_max_sq;

  if (fit_cubic_to_points(
    points, points_offset, points_offset_len,
    tan_l, tan_r,
    (calc_flag & CURVE_FIT_CALC_HIGH_QUALIY) ? FLT_EPSILON : error_threshold_sq,
    cubic, &error_max_sq, &split_index) ||
    (error_max_sq < error_threshold_sq))
  {
    curves.insert(curves.begin(), cubic);
    return;
  }

  vec2 tan_center;

  vec2 pt_a = points[split_index - 1].position;
  vec2 pt_b = points[split_index + 1].position;

  assert(split_index < points.size());
  if (pt_a == pt_b) {
    pt_a = points[split_index].position;
  }

  {
    const vec2 pt = points[split_index].position;

    vec2 tan_center_a = normalize(pt_a - pt);
    vec2 tan_center_b = normalize(pt - pt_b);

    normalize(tan_center_a + tan_center_b, tan_center);
  }

  fit_cubic_to_points_recursive(
    points, points_offset, split_index + 1,
    tan_l, tan_center, error_threshold_sq, calc_flag, curves
  );
  fit_cubic_to_points_recursive(
    points, split_index, points_offset_len - split_index,
    tan_center, tan_r, error_threshold_sq, calc_flag, curves
  );
}

static std::vector<Cubic> curve_fit_cubic_to_points(
  std::vector<FreehandPathPoint>& points,
  float error_threshold,
  const uint calc_flag
) {
  const uint dims = 2;
  const uint points_len = (uint)points.size();

  std::vector<Cubic> curves{};

  uint corners[2] = { 0, points_len - 1 };
  uint corners_len = 2;

  vec2 tan_l, tan_r;

  // uint* corner_index_array = NULL;
  uint corner_index = 0;

  const float error_threshold_sq = error_threshold * error_threshold;

  for (uint i = 1; i < corners_len; i++) {
    const uint points_offset_len = corners[i] - corners[i - 1] + 1;
    const uint first_point = corners[i - 1];

    assert(points_offset_len >= 1);
    if (points_offset_len > 1) {
      const FreehandPathPoint& pt_l = points[first_point];
      const FreehandPathPoint& pt_r = points[first_point + points_offset_len - 1];
      const FreehandPathPoint& pt_l_next = points[first_point + 1];
      const FreehandPathPoint& pt_r_prev = points[first_point + points_offset_len - 2];

      normalize(pt_l.position - pt_l_next.position, tan_l);
      normalize(pt_r_prev.position - pt_r.position, tan_r);

      fit_cubic_to_points_recursive(
        points, first_point, points_offset_len,
        tan_l, tan_r, error_threshold_sq, calc_flag, curves
      );
    } else if (points_len == 1) {
      assert(points_offset_len == 1);
      assert(corners_len == 2);
      assert(corners[0] == 0);
      assert(corners[1] == 0);

      const vec2 pt = points[0].position;

      Cubic cubic{ pt, pt, pt, pt };
      curves.insert(curves.begin(), cubic);
    }

    // if (corner_index_array) {
    //   corner_index_array[corner_index++] = curves.size();
    // }
  }

  return curves;
}
