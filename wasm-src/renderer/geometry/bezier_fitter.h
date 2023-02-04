/*
 * Original implementation:
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 */

#pragma once

#include "../../math/vector.h"
#include "../../math/matrix.h"
#include "../../utils/console.h"

#include <vector>

#define MAXPOINTS	1000

 // TODO: Move
struct FreehandPathPoint {
  vec2 position;
  float pressure;
};

using PathPoints = std::vector<FreehandPathPoint>;

struct Bezier {
  vec2 p0;
  vec2 p1;
  vec2 p2;
  vec2 p3;

  vec2& operator[](uint8_t i) {
    switch (i) {
    default:
    case 0:
      return p0;
    case 1:
      return p1;
    case 2:
      return p2;
    case 3:
      return p3;
    }
  }

  const vec2& operator[](uint8_t i) const {
    switch (i) {
    default:
    case 0:
      return p0;
    case 1:
      return p1;
    case 2:
      return p2;
    case 3:
      return p3;
    }
  }
};

static const vec2* operator&(const Bezier& b) { return (vec2*)&(b.p0); }

/*
 * B0, B1, B2, B3 :
 * Bezier multipliers
 */
static float B0(float u) {
  float tmp = 1.0f - u;
  return (tmp * tmp * tmp);
}

static float B1(float u) {
  float tmp = 1.0f - u;
  return (3.0f * u * (tmp * tmp));
}

static float B2(float u) {
  float tmp = 1.0f - u;
  return (3.0f * u * u * tmp);
}

static float B3(float u) {
  return (u * u * u);
}

/*
 * generate_bezier:
 * Use least-squares method to find Bezier control points for region.
 */
static Bezier generate_bezier(
  const PathPoints& points, const size_t first, const size_t last,
  const std::vector<float>& u_prime,
  const vec2& t_hat_1, const vec2& t_hat_2
) {
  size_t i;
  vec2 A[MAXPOINTS][2];	/* Precomputed rhs for eqn */
  size_t n_pts; /* Number of pts in sub-curve */
  mat2 C{ 0.0f }; /* Matrix C */
  vec2 X{ 0.0f }; /* Matrix X */
  float det_C0_C1, det_C0_X, det_X_C1; /* Determinants of matrices */
  float alpha_l, alpha_r; /* Alpha values, left and right	*/
  vec2 tmp{};			/* Utility variable	*/
  Bezier bez_curve{};	/* RETURN bezier curve ctl pts */
  float seg_length;
  float epsilon;

  n_pts = last - first + 1;

  /* Compute the A's*/
  for (i = 0; i < n_pts; i++) {
    A[i][0] = t_hat_1 * B1(u_prime[i]);
    A[i][1] = t_hat_2 * B2(u_prime[i]);
  }

  for (i = 0; i < n_pts; i++) {
    C[0][0] += dot(A[i][0], A[i][0]);
    C[0][1] += dot(A[i][0], A[i][1]);
    C[1][0] = C[0][1];
    C[1][1] += dot(A[i][1], A[i][1]);

    tmp = points[first + i].position - (
      points[first].position * B0(u_prime[i]) +
      points[first].position * B1(u_prime[i]) +
      points[last].position * B2(u_prime[i]) + points[last].position * B3(u_prime[i]));

    X[0] += dot(A[i][0], tmp);
    X[1] += dot(A[i][1], tmp);
  }

  /* Compute the determinants of C and X (Cramer)	*/
  det_C0_C1 = determinant(C);
  det_C0_X = C[0][0] * X[1] - C[1][0] * X[0];
  det_X_C1 = X[0] * C[1][1] - X[1] * C[0][1];

  /* Finally, derive alpha values	*/
  alpha_l = (det_C0_C1 == 0.0f) ? 0.0f : det_X_C1 / det_C0_C1;
  alpha_r = (det_C0_C1 == 0.0f) ? 0.0f : det_C0_X / det_C0_C1;

  /* If alpha negative, use the Wu/Barsky heuristic (see text)
   * (if alpha is 0, you get coincident control points that lead to
   * divide by zero in any subsequent NewtonRaphsonRootFind() call. */
  seg_length = distance(points[last].position, points[first].position);
  epsilon = 1.0e-6f * seg_length;
  if (alpha_l < epsilon || alpha_r < epsilon) {
    /* fall back on standard (probably inaccurate) formula, and subdivide further if needed. */
    float dist = seg_length / 3.0f;

    bez_curve.p0 = points[first].position;
    bez_curve.p3 = points[last].position;
    bez_curve.p1 = bez_curve.p0 + t_hat_1 * dist;
    bez_curve.p2 = bez_curve.p3 + t_hat_2 * dist;

    return bez_curve;
  }

  /* First and last control points of the Bezier curve are */
  /* positioned exactly at the first and last data points */
  /* Control points 1 and 2 are positioned an alpha distance out */
  /* on the tangent vectors, left and right, respectively */
  bez_curve.p0 = points[first].position;
  bez_curve.p3 = points[last].position;
  bez_curve.p1 = bez_curve.p0 + t_hat_1 * alpha_l;
  bez_curve.p2 = bez_curve.p3 + t_hat_2 * alpha_r;
  return (bez_curve);
}

/*
 * Bezier:
 * Evaluate a Bezier curve at a particular parameter value
 */
static vec2 BII(int degree, const vec2* V, float t) {
  int i, j;
  vec2 Q;
  vec2* V_temp;

  V_temp = new vec2[degree + 1];

  for (i = 0; i <= degree; i++) {
    V_temp[i] = V[i];
  }

  /* Triangle computation	*/
  for (i = 1; i <= degree; i++) {
    for (j = 0; j <= degree - i; j++) {
      V_temp[j].x = (1.0f - t) * V_temp[j].x + t * V_temp[j + 1].x;
      V_temp[j].y = (1.0f - t) * V_temp[j].y + t * V_temp[j + 1].y;
    }
  }

  Q = V_temp[0];
  delete[] V_temp;

  return V_temp[0];
}

/*
 * newton_raphson_root_find :
 * Use Newton-Raphson iteration to find better root.
 */
static float newton_raphson_root_find(const Bezier& Q, const vec2& P, const float u) {
  float numerator, denominator;
  vec2 Q1[3], Q2[2]; /* Q' and Q'' */
  vec2 Q_u, Q1_u, Q2_u; /* u evaluated at Q, Q', & Q'' */
  float u_prime; /* Improved u */

  /* Compute Q(u)	*/
  Q_u = BII(3, &Q, u);

  /* Generate control vertices for Q'	*/
  for (int i = 0; i <= 2; i++) {
    Q1[i].x = (Q[i + 1].x - Q[i].x) * 3.0f;
    Q1[i].y = (Q[i + 1].y - Q[i].y) * 3.0f;
  }

  /* Generate control vertices for Q'' */
  for (int i = 0; i <= 1; i++) {
    Q2[i].x = (Q1[i + 1].x - Q1[i].x) * 2.0f;
    Q2[i].y = (Q1[i + 1].y - Q1[i].y) * 2.0f;
  }

  /* Compute Q'(u) and Q''(u)	*/
  Q1_u = BII(2, Q1, u);
  Q2_u = BII(1, Q2, u);

  /* Compute f(u)/f'(u) */
  numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
  denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
    (Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);
  if (denominator == 0.0f) return u;

  /* u = u - f(u)/f'(u) */
  u_prime = u - (numerator / denominator);
  return u_prime;
}

/*
 * reparameterize:
 * Given set of points and their parameterization, try to find
 * a better parameterization.
 */
static std::vector<float> reparameterize(
  const PathPoints& points, const size_t first, const size_t last,
  const std::vector<float>& u, const Bezier& bez_curve
) {
  size_t n_pts = last - first + 1;
  std::vector<float> u_prime(n_pts); /* New parameter values */

  for (size_t i = first; i <= last; i++) {
    u_prime[i - first] = newton_raphson_root_find(bez_curve, points[i].position, u[i - first]);
  }
  return u_prime;
}

/*
 * compute_left_tangent, compute_right_tangent, compute_center_tangent:
 * Approximate unit tangents at endpoints and "center" of digitized curve
 */
static vec2 compute_left_tangent(const PathPoints& points, size_t end) {
  vec2 t_hat_1;

  t_hat_1 = points[end + 1].position - points[end].position;
  t_hat_1 = normalize(t_hat_1);

  return t_hat_1;
}

static vec2 compute_right_tangent(const PathPoints& points, size_t end) {
  vec2 t_hat_2;

  t_hat_2 = points[end - 1].position - points[end].position;
  t_hat_2 = normalize(t_hat_2);

  return t_hat_2;
}


static vec2 compute_center_tangent(const PathPoints& points, size_t center) {
  vec2	v1, v2, t_hat_center;

  v1 = points[center - 1].position - points[center].position;
  v2 = points[center].position - points[center + 1].position;

  t_hat_center = (v1 + v2) / 2.0f;
  t_hat_center = normalize(t_hat_center);

  return t_hat_center;
}


/*
 * chord_length_parameterize:
 * Assign parameter values to digitized points
 * using relative distances between points.
 */
static std::vector<float> chord_length_parameterize(const PathPoints& points, size_t first, size_t last) {
  std::vector<float> u(last - first + 1); /* Parameterization */
  size_t i;

  u[0] = 0.0f;
  for (i = first + 1; i <= last; i++) {
    u[i - first] = u[i - first - 1] + distance(points[i].position, points[i - 1].position);
  }

  for (i = first + 1; i <= last; i++) {
    u[i - first] = u[i - first] / u[last - first];
  }

  return u;
}

/*
 * compute_max_error:
 * Find the maximum squared distance of digitized points
 * to fitted curve.
 */
static float compute_max_error(
  const PathPoints& points, const size_t first, const size_t last,
  const Bezier& bez_curve, const std::vector<float>& u, size_t* split_point
) {
  size_t i;
  float	max_dist; /* Maximum error */
  float	dist; /* Current error */
  vec2 P; /* Point on curve	*/
  vec2 v; /* Vector from point to curve	*/

  *split_point = (last - first + 1) / 2;
  max_dist = 0.0f;

  for (i = first + 1; i < last; i++) {
    P = BII(3, &bez_curve, u[i - first]);
    v = P - points[i].position;
    dist = squared_length(v);

    if (dist >= max_dist) {
      max_dist = dist;
      *split_point = i;
    }
  }

  return max_dist;
}

static void fit_cubic(
  const PathPoints& points, size_t first, size_t last,
  vec2 t_hat_1, vec2 t_hat_2, float error,
  std::vector<Bezier>& r_curves
) {
  Bezier bez_curve; /* Control points of fitted Bezier curve */
  std::vector<float> u; /* Parameter values for point */
  std::vector<float> u_prime; /* Improved parameter values */
  float	max_error; /* Maximum fitting error */
  size_t split_point; /* Point to split point set at */
  size_t n_pts = last - first + 1; /* Number of points in subset */
  float	iteration_error = error * 4.0f; /* Error below which you try iterating */
  int max_iterations = 4; /* Max times to try iterating */
  vec2 t_hat_center; /* Unit tangent vector at split_point */

  /* Use heuristic if region only has two points in it */
  if (n_pts == 2) {
    float dist = distance(points[last].position, points[first].position) / 3.0f;

    bez_curve.p0 = points[first].position;
    bez_curve.p3 = points[last].position;
    bez_curve.p1 = bez_curve.p0 + t_hat_1 * dist;
    bez_curve.p2 = bez_curve.p3 + t_hat_2 * dist;

    r_curves.push_back(bez_curve);

    return;
  }

  /* Parameterize points, and attempt to fit curve */
  u = chord_length_parameterize(points, first, last);
  bez_curve = generate_bezier(points, first, last, u, t_hat_1, t_hat_2);

  /* Find max deviation of points to fitted curve */
  max_error = compute_max_error(points, first, last, bez_curve, u, &split_point);
  if (max_error < error) {
    r_curves.push_back(bez_curve);
    return;
  }

  /* If error not too large, try some reparameterization
   * and iteration
   */
  if (max_error < iteration_error) {
    for (size_t i = 0; i < max_iterations; i++) {
      u_prime = reparameterize(points, first, last, u, bez_curve);
      bez_curve = generate_bezier(points, first, last, u_prime, t_hat_1, t_hat_2);
      max_error = compute_max_error(points, first, last, bez_curve, u_prime, &split_point);

      if (max_error < error) {
        r_curves.push_back(bez_curve);
        return;
      }

      u.swap(u_prime);
    }
  }

  /* Fitting failed -- split at max error point and fit recursively */
  t_hat_center = compute_center_tangent(points, split_point);
  fit_cubic(points, first, split_point, t_hat_1, t_hat_center, error, r_curves);
  negate(t_hat_center, t_hat_center);
  fit_cubic(points, split_point, last, t_hat_center, t_hat_2, error, r_curves);
}

static std::vector<Bezier> fit_to_bezier_curves(const PathPoints& points, const size_t start, const size_t end, const float error) {
  // std::vector<std::vector<float>> parameterization = initialize_parameterization(points, start, end);

  vec2 t_hat_1 = compute_left_tangent(points, start);
  vec2 t_hat_2 = compute_right_tangent(points, end);

  std::vector<Bezier> curves{};

  fit_cubic(points, start, end, t_hat_1, t_hat_2, error, curves);

  return curves;
}
