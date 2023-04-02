#include "path_fitter.h"

#include "../mat2.h"
#include "../vector.h"
#include "../matrix.h"
#include "path_simplifier.h"
#include "../../utils/defines.h"
#include "../../utils/console.h"

#define MAX_POINTS	1000

static const vec2* operator&(const PathBezier& b) { return (vec2*)&(b.p0); }

/*
 * B0, B1, B2, B3 :
 * PathBezier multipliers
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
 * Use least-squares method to find PathBezier control points for region.
 */
static PathBezier generate_bezier(
  const std::vector<PathPoint>& points, const size_t first, const size_t last,
  const std::vector<float>& u_prime,
  const vec2& t_hat_1, const vec2& t_hat_2
) {
  size_t i;
  vec2 A[MAX_POINTS][2];	/* Precomputed rhs for eqn */
  size_t n_pts; /* Number of pts in sub-curve */
  mat2 C{ 0.0f }; /* Matrix C */
  vec2 X{ 0.0f }; /* Matrix X */
  float det_C0_C1, det_C0_X, det_X_C1; /* Determinants of matrices */
  float alpha_l, alpha_r; /* Alpha values, left and right	*/
  vec2 tmp;			/* Utility variable	*/
  PathBezier bez_curve;	/* RETURN bezier curve ctl pts */
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

    bez_curve.start_index = first;
    bez_curve.end_index = last;

    return bez_curve;
  }

  /* First and last control points of the PathBezier curve are */
  /* positioned exactly at the first and last data points */
  /* Control points 1 and 2 are positioned an alpha distance out */
  /* on the tangent vectors, left and right, respectively */
  bez_curve.p0 = points[first].position;
  bez_curve.p3 = points[last].position;
  bez_curve.p1 = bez_curve.p0 + t_hat_1 * alpha_l;
  bez_curve.p2 = bez_curve.p3 + t_hat_2 * alpha_r;

  bez_curve.start_index = first;
  bez_curve.end_index = last;

  bez_curve.pressure.x = points[first].pressure;
  bez_curve.pressure.y = points[last].pressure;

#if 0
  uint half = (last - first) / 2;

  for (int i = 1; i < half; i++) {
    bez_curve.pressure.x = (bez_curve.pressure.x * half + points[first + i].pressure * (half - i)) / (half + half - i);
    bez_curve.pressure.y = (bez_curve.pressure.y * half + points[last - i].pressure * (half - i)) / (half + half - i);
  }
#endif

  return (bez_curve);
}

/*
 * PathBezier:
 * Evaluate a PathBezier curve at a particular parameter value
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

  return Q;
}

/*
 * newton_raphson_root_find :
 * Use Newton-Raphson iteration to find better root.
 */
static float newton_raphson_root_find(const PathBezier& Q, const vec2& P, const float u) {
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
  const std::vector<PathPoint>& points, const size_t first, const size_t last,
  const std::vector<float>& u, const PathBezier& bez_curve
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
static vec2 compute_left_tangent(const std::vector<PathPoint>& points, size_t end) {
  vec2 t_hat_1;

  t_hat_1 = points[end + 1].position - points[end].position;
  t_hat_1 = normalize(t_hat_1);

  return t_hat_1;
}

static vec2 compute_right_tangent(const std::vector<PathPoint>& points, size_t end) {
  vec2 t_hat_2;

  t_hat_2 = points[end - 1].position - points[end].position;
  t_hat_2 = normalize(t_hat_2);

  return t_hat_2;
}


static vec2 compute_center_tangent(const std::vector<PathPoint>& points, size_t center) {
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
static std::vector<float> chord_length_parameterize(const std::vector<PathPoint>& points, size_t first, size_t last) {
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
  const std::vector<PathPoint>& points, const size_t first, const size_t last,
  const PathBezier& bez_curve, const std::vector<float>& u, size_t* split_point
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

void fit_cubic(
  const std::vector<PathPoint>& points, size_t first, size_t last,
  vec2 t_hat_1, vec2 t_hat_2, float error,
  std::vector<PathBezier>& r_curves
) {
  PathBezier bez_curve; /* Control points of fitted PathBezier curve */
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
    bez_curve.p1 = bez_curve.p0;
    bez_curve.p2 = bez_curve.p3;

    bez_curve.start_index = first;
    bez_curve.end_index = last;

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

    if (max_error > 1) {
      console::log("error", max_error);
    }
    if (max_error > 6) {
      console::log("error", max_error);
    }
  }

  /* Fitting failed -- split at max error point and fit recursively */
  t_hat_center = compute_center_tangent(points, split_point);
  fit_cubic(points, first, split_point, t_hat_1, t_hat_center, error, r_curves);
  negate(t_hat_center, t_hat_center);
  fit_cubic(points, split_point, last, t_hat_center, t_hat_2, error, r_curves);
}

void fit_path(
  const std::vector<PathPoint>& points,
  const uint start_index,
  const uint end_index,
  const float error,
  std::vector<PathBezier>& beziers
) {
  vec2 t_hat_1 = compute_left_tangent(points, start_index);
  vec2 t_hat_2 = compute_right_tangent(points, end_index);

  fit_cubic(points, start_index, end_index, t_hat_1, t_hat_2, error, beziers);
}

void refit_cubic(
  const std::vector<PathPoint>& points, size_t first, size_t last,
  vec2 t_hat_1, vec2 t_hat_2, float error,
  std::vector<PathBezier>& r_curves
) {
  PathBezier bez_curve; /* Control points of fitted PathBezier curve */
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
    bez_curve.p1 = bez_curve.p0;
    bez_curve.p2 = bez_curve.p3;

    bez_curve.start_index = first;
    bez_curve.end_index = last;

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
   // if (max_error < iteration_error) {
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
  // }

  /* Fitting failed -- split at max error point and fit recursively */
  // t_hat_center = compute_center_tangent(points, split_point);
  // fit_cubic(points, first, split_point, t_hat_1, t_hat_center, error, r_curves);
  // negate(t_hat_center, t_hat_center);
  // fit_cubic(points, split_point, last, t_hat_center, t_hat_2, error, r_curves);

  // TEMP: Temp solution to avoid missing segments
  {
    float dist = distance(points[last].position, points[first].position) / 3.0f;

    bez_curve.p0 = points[first].position;
    bez_curve.p3 = points[last].position;
    bez_curve.p1 = bez_curve.p0;
    bez_curve.p2 = bez_curve.p3;

    bez_curve.start_index = first;
    bez_curve.end_index = last;

    r_curves.push_back(bez_curve);
  }
}

static PathBezier fit_bezier(
  const std::vector<PathPoint>& points,
  const uint start_index,
  const uint end_index,
  const float sq_error
) {

}

#if 1
void refit_path(
  const std::vector<PathPoint>& points,
  const uint start_index,
  const uint end_index,
  const float error,
  std::vector<PathBezier>& beziers
) {
  std::vector<uint> indices = simplify_path(points, start_index, end_index, error * 30.0f);
  // std::vector<vec2> tangents;
  std::vector<vec2> left_tangents(indices.size());
  std::vector<vec2> right_tangents(indices.size());

  vec2 t_hat_1 = points[indices[indices.size() - 1] - 1].position - points[indices[indices.size() - 1]].position;
  vec2 t_hat_2 = points[indices[0] + 1].position - points[indices[0]].position;

  normalize(t_hat_1, t_hat_1);
  normalize(t_hat_2, t_hat_2);

  left_tangents[indices.size() - 1] = t_hat_1;
  right_tangents[0] = t_hat_2;

  for (int i = 1; i < indices.size() - 1; i++) {
    // vec2 t_hat_1 = compute_left_tangent(points, indices[i]);
    // vec2 t_hat_2 = compute_right_tangent(points, indices[i + 1]);

    // if (tangents.empty()) {
    t_hat_1 = points[indices[i] - 1].position - points[indices[i]].position;
    normalize(t_hat_1, t_hat_1);

    left_tangents[i] = t_hat_1;
    // } else {
    //   vec2& t_hat_last = tangents.back();
    //   vec2 t_hat = midpoint(-t_hat_last, t_hat_1);
    //   normalize(t_hat, t_hat);

    //   t_hat_last = t_hat;
    // }

    t_hat_2 = points[indices[i] + 1].position - points[indices[i]].position;
    normalize(t_hat_2, t_hat_2);

    right_tangents[i] = t_hat_2;
  }

  // if (start_index == 0) {
  // {
  //   vec2 start = points[indices.front()].position;
  //   vec2 end = points[indices.front() + 1].position;
  //   vec2 t_hat = end - start;
  //   vec2 other;

  //   float sq_len = squared_length(t_hat);
  //   int count = 2;

  //   while (sq_len < GEOMETRY_SQR_EPSILON && indices[1] - indices.front() > count + 1) {
  //     other = points[indices.front() + count].position;
  //     end = (end * (count - 1) + other) / count;

  //     t_hat = end - start;
  //     sq_len = squared_length(end - start);

  //     count++;
  //   }

  //   tangents.front() = t_hat / std::sqrtf(sq_len) * 1.0f;
  // }
  // }

  // if (end_index == points.size() - 1) {
  // {
  //   vec2 end = points[indices.back()].position;
  //   vec2 start = points[indices.back() - 1].position;
  //   vec2 t_hat = end - start;
  //   vec2 other;

  //   float sq_len = squared_length(t_hat);
  //   int count = 2;

  //   while (sq_len < GEOMETRY_SQR_EPSILON && indices.back() - indices[indices.size() - 2] > count + 1) {
  //     other = points[indices.back() - count].position;
  //     start = (start * (count - 1) + other) / count;

  //     t_hat = end - start;
  //     sq_len = squared_length(end - start);

  //     count++;
  //   }

  //   tangents.back() = -t_hat / std::sqrtf(sq_len) * 3.0f;
  // }
  // }

  for (int j = 0; j < indices.size() - 1; j++) {
    PathBezier bez;

    uint start = indices[j];
    uint end = indices[j + 1];

    vec2 p0 = points[start].position;
    vec2 p3 = points[end].position;

    bez.p0 = bez.p1 = p0;
    bez.p3 = bez.p2 = p3;

    float a = 0.0f, b = 0.0f;

    if (end - start > 3) {
      std::vector<vec2> values(end - start - 1);

      for (int i = start + 1; i < end; i++) {
        vec2 q = points[i].position;

        float t = (float)(i - start) / (float)(end - start);
        float A = std::powf(1 - t, 3);
        float B = 3 * t * std::powf(1 - t, 2);
        float C = 3 * t * t * (1 - t);
        float D = t * t * t;

        vec2 tau1 = right_tangents[j];
        vec2 tau2 = left_tangents[j + 1];

        vec2 omega = (A + B) * p0 + (C + D) * p3;
        float sigma = tau2.x / tau2.y;

        a = (-omega.x + q.x + sigma * (omega.y - q.y)) / (B * tau1.x * (1 - sigma * B * tau1.y));
        b = -(omega.y + B * a * tau1.y - q.y) / (C * tau2.y);

        values[i - start - 1] = vec2(a, b);
      }

      a = b = 0.0f;

      for (vec2 v : values) {
        a += v.x;
        b += v.y;
      }

      a /= values.size();
      b /= values.size();

      console::log("a", a);
    }

    bez.p1 += a * right_tangents[j];
    bez.p2 += b * left_tangents[j + 1];

    bez.start_index = start;
    bez.end_index = end;

    beziers.push_back(bez);
  }
}
#else
void refit_path(
  const std::vector<PathPoint>& points,
  const uint start_index,
  const uint end_index,
  const float error,
  std::vector<PathBezier>& beziers
) {
  std::vector<uint> indices = simplify_path(points, start_index, end_index, error * 30.0f);
  std::vector<vec2> tangents;

  for (int i = 0; i < indices.size() - 1; i++) {
    vec2 t_hat_1 = compute_left_tangent(points, indices[i]);
    vec2 t_hat_2 = compute_right_tangent(points, indices[i + 1]);

    if (tangents.empty()) {
      tangents.push_back(t_hat_1);
    } else {
      vec2& t_hat_last = tangents.back();
      vec2 t_hat = midpoint(-t_hat_last, t_hat_1);
      normalize(t_hat, t_hat);

      t_hat_last = t_hat;
    }

    tangents.push_back(t_hat_2);
  }

  if (start_index == 0) {
    vec2 start = points[indices.front()].position;
    vec2 end = points[indices.front() + 1].position;
    vec2 t_hat = end - start;
    vec2 other;

    float sq_len = squared_length(t_hat);
    int count = 2;

    while (sq_len < GEOMETRY_SQR_EPSILON && indices[1] - indices.front() > count + 1) {
      other = points[indices.front() + count].position;
      end = (end * (count - 1) + other) / count;

      t_hat = end - start;
      sq_len = squared_length(end - start);

      count++;
    }

    tangents.front() = t_hat / std::sqrtf(sq_len);
  }

  if (end_index == points.size() - 1) {
    vec2 end = points[indices.back()].position;
    vec2 start = points[indices.back() - 1].position;
    vec2 t_hat = end - start;
    vec2 other;

    float sq_len = squared_length(t_hat);
    int count = 2;

    while (sq_len < GEOMETRY_SQR_EPSILON && indices.back() - indices[indices.size() - 2] > count + 1) {
      other = points[indices.back() - count].position;
      start = (start * (count - 1) + other) / count;

      t_hat = end - start;
      sq_len = squared_length(end - start);

      count++;
    }

    tangents.back() = t_hat / std::sqrtf(sq_len);
  }

  for (int i = 0; i < indices.size() - 1; i++) {
    PathBezier bez;

    uint start = indices[i];
    uint end = indices[i + 1];

    vec2 p0 = points[start].position;
    vec2 p3 = points[end].position;

#if 1
    fit_cubic(points, indices[i], indices[i + 1], tangents[i], -tangents[i + 1], error, beziers);
#else

    if (end - start > 2) {
      std::vector<float> running_length(end - start);
      float total_length = 0.0f;

      for (int h = start + 1; h <= end; h++) {
        float len = distance(points[h].position, points[h - 1].position);
        total_length += len;
        running_length[h - start - 1] = total_length;
      }

      vec2 ta = tangents[i];
      vec2 tb = -tangents[i + 1];
      std::vector<vec2> coefficients(end - start - 1);

      for (int h = start + 1; h < end; h++) {
        float t = running_length[h - start - 1] / total_length;

        // float t = float(end - start) / (float)h;
        vec2 gradient{ 0.0f };

        float d1 = distance(p0, points[h].position);
        float d2 = distance(p3, points[h].position);

        float new_t = d1 / (d1 + d2);

        float u = (1 - new_t) * (1 - new_t) / (new_t * new_t + (1 - new_t) * (1 - new_t));
        float ratio = std::abs((new_t * new_t + (1 - new_t) * (1 - new_t) - 1) / (new_t * new_t + (1 - new_t) * (1 - new_t)));

        vec2 C = u * p0 + (1 - u) * p3;
        vec2 A = points[h].position + (points[h].position - C) / ratio;

        coefficients[h - start - 1].x = 0.66f * distance(A, p0);
        coefficients[h - start - 1].y = 0.66f * distance(A, p3);

        // gradient.x = 6 * a * (1 - t) * (1 - t) * t * (-points[h].position.x + p0.x * (1 - t) * (1 - t) * (1 - t) + p3.x * t * t * t + 3 * (1 - t) * (1 - t) * t * (p0.x + a * ta.x) + 3 * (1 - t) * t * t * (p3.x + b * tb.x));
        // gradient.y = 6 * b * (1 - t) * t * t * (-points[h].position.x + p0.x * (1 - t) * (1 - t) * (1 - t) + p3.x * t * t * t + 3 * (1 - t) * (1 - t) * t * (p0.x + a * ta.x) + 3 * (1 - t) * t * t * (p3.x + b * tb.x));

        // float dist = squared_distance(bezier(p0, p0 + ta, p3 + tb, p3, t), points[h].position);

        // console::log("dist", dist);
      }

      vec2 total{ 0.0f };

      for (vec2 coefficient : coefficients) {
        total.x += coefficient.x;
        total.y += coefficient.y;
      }

      vec2 average = total / float(coefficients.size());




      bez.p0 = p0;
      bez.p1 = p0 + average.x * ta;
      bez.p2 = p3 + average.y * tb;
      bez.p3 = p3;

      bez.start_index = indices[i];
      bez.end_index = indices[i + 1];

      beziers.push_back(bez);
    } else {
      bez.p0 = bez.p1 = p0;
      bez.p3 = bez.p2 = p3;

      bez.start_index = indices[i];
      bez.end_index = indices[i + 1];

      beziers.push_back(bez);
    }
#endif
  }

  // fit_cubic(points, start_index, end_index, t_hat_1, t_hat_2, error, beziers);
}
#endif
