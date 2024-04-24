/**
 * @file algorithms/fit.cpp
 * @brief Implementation of the least-squares method to find CubicBezier control points for a given set of points.
 *
 * This file contains the implementation of the least-squares method to find CubicBezier control points for a given set of points.
 * The method uses the B-spline basis functions to generate the control points for a cubic Bezier curve that passes through the given set of points.
 * The implementation includes functions to calculate the B-spline basis functions and to generate the control points for a sub-curve.
 */

#include "fit.h"

#include "../math/matrix.h"

#define MAX_POINTS 1000

namespace graphick::algorithms {

  /**
   * @brief Calculates the value of the B-spline basis function of degree 0 at the given parameter value.
   *
   * @param u The parameter value.
   * @return The value of the B-spline basis function of degree 0 at the given parameter value.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static T B0(const T u) {
    T tmp = T(1) - u;
    return (tmp * tmp * tmp);
  }

  /**
   * @brief Calculates the first basis function for the cubic B-spline curve.
   *
   * @param u The parameter value.
   * @return The value of the first basis function at the given parameter value.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static T B1(const T u) {
    T tmp = T(1) - u;
    return (T(3) * u * (tmp * tmp));
  }

  /**
   * @brief Calculates the second basis function for a cubic B-spline.
   *
   * @param u The parameter value to evaluate the basis function at.
   * @return The value of the second basis function at the given parameter value.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static T B2(const T u) {
    T tmp = T(1) - u;
    return (T(3) * u * u * tmp);
  }

  /**
   * @brief Calculates the cubic B-spline basis function of degree 3 at the given parameter value.
   *
   * @param u The parameter value.
   * @return The value of the cubic B-spline basis function at the given parameter value.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static T B3(const T u) {
    return (u * u * u);
  }

  /**
   * @brief Use least-squares method to find CubicBezier control points for region.
   *
   * @param points The set of control points.
   * @param first The index of the first control point in the set.
   * @param last The index of the last control point in the set.
   * @param u_prime The set of tangent vectors for each control point.
   * @param t_hat_1 The tangent vector at the first control point.
   * @param t_hat_2 The tangent vector at the last control point.
   * @return A cubic Bezier curve.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static geom::CubicBezier<T> generate_bezier(
    const std::vector<math::Vec2<T>>& points,
    const size_t first, const size_t last,
    const std::vector<T>& u_prime,
    const math::Vec2<T> t_hat_1, const math::Vec2<T> t_hat_2
  ) {
    size_t i;
    size_t n_pts;                               /* Number of pts in sub-curve */

    math::Mat2<T> C = math::Mat2<T>::zero();    /* Matrix C */
    math::Vec2<T> X = math::Vec2<T>::zero();    /* Matrix X */
    math::Vec2<T> A[MAX_POINTS][2];             /* Precomputed rhs for eqn */

    T det_C0_C1, det_C0_X, det_X_C1;            /* Determinants of matrices */
    T alpha_l, alpha_r;                         /* Alpha values, left and right	*/

    math::Vec2<T> tmp;                          /* Utility variable	*/
    geom::CubicBezier<T> bez_curve;             /* Return bezier curve ctl pts */

    T seg_length;
    T epsilon;

    n_pts = last - first + 1;

    // Compute the A's.
    for (i = 0; i < n_pts; i++) {
      A[i][0] = t_hat_1 * B1(u_prime[i]);
      A[i][1] = t_hat_2 * B2(u_prime[i]);
    }

    for (i = 0; i < n_pts; i++) {
      C[0][0] += math::dot(A[i][0], A[i][0]);
      C[0][1] += math::dot(A[i][0], A[i][1]);
      C[1][0] = C[0][1];
      C[1][1] += math::dot(A[i][1], A[i][1]);

      tmp = points[first + i] - (
        points[first] * B0(u_prime[i]) +
        points[first] * B1(u_prime[i]) +
        points[last] * B2(u_prime[i]) + points[last] * B3(u_prime[i]));

      X[0] += math::dot(A[i][0], tmp);
      X[1] += math::dot(A[i][1], tmp);
    }

    // Compute the determinants of C and X (Cramer).
    det_C0_C1 = math::determinant(C);
    det_C0_X = C[0][0] * X[1] - C[1][0] * X[0];
    det_X_C1 = X[0] * C[1][1] - X[1] * C[0][1];

    // Finally, derive alpha values.
    alpha_l = math::is_almost_zero(det_C0_C1) ? T(0) : det_X_C1 / det_C0_C1;
    alpha_r = math::is_almost_zero(det_C0_C1) ? T(0) : det_C0_X / det_C0_C1;

    // If alpha negative, use the Wu/Barsky heuristic (see text)
    // (if alpha is 0, you get coincident control points that lead to
    // divide by zero in any subsequent NewtonRaphsonRootFind() call.
    seg_length = math::distance(points[last], points[first]);
    epsilon = T(1e-6) * seg_length;

    if (alpha_l < epsilon || alpha_r < epsilon) {
      // Fall back on standard (probably inaccurate) formula, and subdivide further if needed.
      T dist = seg_length / T(3);

      bez_curve.p0 = points[first];
      bez_curve.p3 = points[last];
      bez_curve.p1 = bez_curve.p0 + t_hat_1 * dist;
      bez_curve.p2 = bez_curve.p3 + t_hat_2 * dist;

      return bez_curve;
    }

    // First and last control points of the CubicBezier curve are
    // positioned exactly at the first and last data points
    // Control points 1 and 2 are positioned an alpha distance out
    // on the tangent vectors, left and right, respectively.
    bez_curve.p0 = points[first];
    bez_curve.p3 = points[last];
    bez_curve.p1 = bez_curve.p0 + t_hat_1 * alpha_l;
    bez_curve.p2 = bez_curve.p3 + t_hat_2 * alpha_r;

    return bez_curve;
  }

  /**
   * @brief Calculates the value of the B-spline basis function of degree `degree` and index `i` at parameter value `t`.
   *
   * @param degree The degree of the B-spline basis function.
   * @param V An array of control points.
   * @param t The parameter value at which to evaluate the basis function.
   * @return The value of the B-spline basis function at parameter value `t`.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static math::Vec2<T> BII(const int degree, const math::Vec2<T>* V, T t) {
    int i, j;
    math::Vec2<T> Q;
    math::Vec2<T>* V_temp;

    V_temp = new math::Vec2<T>[degree + 1];

    for (i = 0; i <= degree; i++) {
      V_temp[i] = V[i];
    }

    // Triangle computation.
    for (i = 1; i <= degree; i++) {
      for (j = 0; j <= degree - i; j++) {
        V_temp[j].x = (T(1) - t) * V_temp[j].x + t * V_temp[j + 1].x;
        V_temp[j].y = (T(1) - t) * V_temp[j].y + t * V_temp[j + 1].y;
      }
    }

    Q = V_temp[0];
    delete[] V_temp;

    return Q;
  }

  /**
   * @brief Computes the root of a cubic bezier curve using the Newton-Raphson method.
   *
   * @param Q The cubic bezier curve.
   * @param P The point to find the root for.
   * @param u The initial guess for the root.
   * @return The root of the cubic bezier curve.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static T newton_raphson_root_find(const geom::CubicBezier<T>& Q, const math::Vec2<T> P, const T u) {
    T numerator, denominator;

    math::Vec2<T> Q1[3], Q2[2];       /* Q' and Q'' */
    math::Vec2<T> Q_u, Q1_u, Q2_u;    /* u evaluated at Q, Q', & Q'' */

    T u_prime;                        /* Improved u */

    // Compute Q(u).
    Q_u = BII(3, &Q, u);

    // Generate control vertices for Q'.
    for (int i = 0; i <= 2; i++) {
      Q1[i].x = (Q[i + 1].x - Q[i].x) * T(3);
      Q1[i].y = (Q[i + 1].y - Q[i].y) * T(3);
    }

    // Generate control vertices for Q''.
    for (int i = 0; i <= 1; i++) {
      Q2[i].x = (Q1[i + 1].x - Q1[i].x) * T(2);
      Q2[i].y = (Q1[i + 1].y - Q1[i].y) * T(2);
    }

    // Compute Q'(u) and Q''(u).
    Q1_u = BII(2, Q1, u);
    Q2_u = BII(1, Q2, u);

    // Compute f(u)/f'(u).
    numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
    denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
      (Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);

    if (math::is_almost_zero(denominator)) return u;

    // u = u - f(u)/f'(u).
    u_prime = u - (numerator / denominator);

    return u_prime;
  }

  /**
   * @brief Tries to find a better parameterization for a given set of points.
   *
   * @param points The set of points to match the curve to.
   * @param first The index of the first point in the section.
   * @param last The index of the last point in the section.
   * @param u The original parameterization.
   * @param bez_curve The cubic Bezier curve to reparameterize.
   * @return A vector of new parameter values for the section.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static std::vector<T> reparameterize(
    const std::vector<math::Vec2<T>>& points,
    const size_t first, const size_t last,
    const std::vector<T>& u, const geom::CubicBezier<T>& bez_curve
  ) {
    size_t n_pts = last - first + 1;
    std::vector<T> u_prime(n_pts);    /* New parameter values */

    for (size_t i = first; i <= last; i++) {
      u_prime[i - first] = newton_raphson_root_find(bez_curve, points[i], u[i - first]);
    }
    return u_prime;
  }

  /**
   * @brief Computes the left tangent of a point on a curve defined by a vector of points.
   *
   * @param points A vector of points defining the curve.
   * @param end The index of the point to compute the left tangent for.
   * @return A math::Vec2<T> representing the left tangent of the specified point.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static math::Vec2<T> compute_left_tangent(const std::vector<math::Vec2<T>>& points, size_t end) {
    math::Vec2<T> t_hat_1;

    t_hat_1 = points[end + 1] - points[end];
    t_hat_1 = math::normalize(t_hat_1);

    return t_hat_1;
  }

  /**
   * @brief Computes the right tangent of a point in a vector of points.
   *
   * @param points The vector of points.
   * @param end The index of the point to compute the right tangent for.
   * @return The right tangent of the specified point.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static math::Vec2<T> compute_right_tangent(const std::vector<math::Vec2<T>>& points, size_t end) {
    math::Vec2<T> t_hat_2;

    t_hat_2 = points[end - 1] - points[end];
    t_hat_2 = math::normalize(t_hat_2);

    return t_hat_2;
  }

  /**
   * @brief Computes the center tangent of a set of points around a given center index.
   *
   * @param points The vector of points to compute the center tangent for.
   * @param center The index of the center point to compute the tangent around.
   * @return The center tangent as a math::Vec2<T>.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static math::Vec2<T> compute_center_tangent(const std::vector<math::Vec2<T>>& points, size_t center) {
    math::Vec2<T>	v1, v2, t_hat_center;

    v1 = points[center - 1] - points[center];
    v2 = points[center] - points[center + 1];

    t_hat_center = (v1 + v2) / T(2);
    t_hat_center = math::normalize(t_hat_center);

    return t_hat_center;
  }

  /**
   * @brief Computes the chord length parameterization for a given set of points.
   *
   * @param points The vector of 2D points to parameterize.
   * @param first The index of the first point to parameterize.
   * @param last The index of the last point to parameterize.
   * @return A vector of Ts representing the chord length parameterization.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static std::vector<T> chord_length_parameterize(const std::vector<math::Vec2<T>>& points, size_t first, size_t last) {
    std::vector<T> u(last - first + 1);    /* Parameterization */
    size_t i;

    u[0] = T(0);
    for (i = first + 1; i <= last; i++) {
      u[i - first] = u[i - first - 1] + math::distance(points[i], points[i - 1]);
    }

    for (i = first + 1; i <= last; i++) {
      u[i - first] = u[i - first] / u[last - first];
    }

    return u;
  }

  /**
   * @brief Computes the maximum error (squared distance) between a cubic Bezier curve and a set of path points.
   *
   * @param points The set of path points to compare against the curve.
   * @param first The index of the first point to compare.
   * @param last The index of the last point to compare.
   * @param bez_curve The cubic Bezier curve to compare against the points.
   * @param u The set of parameter values for the curve.
   * @param split_point A pointer to the index of the point where the curve should be split (maximum error index).
   * @return The maximum error between the curve and the points.
   */
  template <typename T, std::enable_if<std::is_floating_point_v<T>>>
  inline static T compute_max_error(
    const std::vector<math::Vec2<T>>& points,
    const size_t first, const size_t last,
    const geom::CubicBezier<T>& bez_curve,
    const std::vector<T>& u, size_t* split_point
  ) {
    size_t i;
    T	max_dist;         /* Maximum error */
    T	dist;             /* Current error */
    math::Vec2<T> P;    /* Point on curve	*/
    math::Vec2<T> v;    /* Vector from point to curve	*/

    *split_point = (last - first + 1) / 2;
    max_dist = T(0);

    for (i = first + 1; i < last; i++) {
      P = BII(3, &bez_curve, u[i - first]);
      v = P - points[i];
      dist = math::squared_length(v);

      if (dist >= max_dist) {
        max_dist = dist;
        *split_point = i;
      }
    }

    return max_dist;
  }

  template <typename T, typename _>
  geom::CubicBezier<T> fit_points_to_cubic(
    const std::vector<math::Vec2<T>>& points,
    const T error
  ) {
    uint32_t first = 0;                                          /* Index of first control point */
    uint32_t last = static_cast<uint32_t>(points.size()) - 1;    /* Index of last control point */

    geom::CubicBezier<T> bez_curve;                              /* Control points of fitted CubicBezier curve */

    std::vector<T> u;                                            /* Parameter values for point */
    std::vector<T> u_prime;                                      /* Improved parameter values */

    T	max_error;                                                 /* Maximum fitting error */
    T	iteration_error = error * T(4);                            /* Error below which you try iterating */
    int max_iterations = 8;                                      /* Max times to try iterating */

    size_t split_point;                                          /* Point to split point set at */
    size_t n_pts = last - first + 1;                             /* Number of points in subset */

    // Unit tangent vectors at endpoints.
    math::Vec2<T> t_hat_1 = compute_left_tangent(points, first);
    math::Vec2<T> t_hat_2 = compute_right_tangent(points, last);

    // Use heuristic if region only has two points in it.
    if (n_pts == 2) {
      T dist = math::distance(points[last], points[first]) / T(3);

      bez_curve.p0 = points[first];
      bez_curve.p3 = points[last];
      bez_curve.p1 = bez_curve.p0;
      bez_curve.p2 = bez_curve.p3;

      return bez_curve;
    }

    // Parameterize points, and attempt to fit curve.
    u = chord_length_parameterize(points, first, last);
    bez_curve = generate_bezier(points, first, last, u, t_hat_1, t_hat_2);

    // Find max deviation of points to fitted curve.
    max_error = compute_max_error(points, first, last, bez_curve, u, &split_point);
    if (max_error < error) {
      return bez_curve;
    }

    // If error not too large, try some reparameterization and iteration.
    for (size_t i = 0; i < max_iterations; i++) {
      u_prime = reparameterize(points, first, last, u, bez_curve);
      bez_curve = generate_bezier(points, first, last, u_prime, t_hat_1, t_hat_2);
      max_error = compute_max_error(points, first, last, bez_curve, u_prime, &split_point);

      if (max_error < error) {
        return bez_curve;
      }

      u.swap(u_prime);
    }

    return bez_curve;
  }

  /* -- Template Instantiation -- */

  template geom::CubicBezier<float> fit_points_to_cubic(const std::vector<math::Vec2<float>>& points, const float tolerance);
  template geom::CubicBezier<double> fit_points_to_cubic(const std::vector<math::Vec2<double>>& points, const double tolerance);

}
