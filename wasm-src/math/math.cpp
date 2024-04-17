/**
 * @file math/math.cpp
 * @brief Contains the implementation of math functions used by the Graphick editor.
 *
 * @todo try inlining lerps in split_bezier()
 * @todo ask if roots with multiplicity > 1 should be considered as separate roots
 */

#include "math.h"

#include <algorithm>

namespace graphick::math {

  QuadraticSolutions<double> solve_quadratic(const double a, const double b, const double c) {
    if (is_almost_zero(a)) {
      /* It is a linear equation */

      return { solve_linear(b, c) };
    }

    const double discriminant = b * b - 4.0 * a * c;

    if (is_almost_zero(discriminant)) {
      /* One real root. */

      const double root = -b / (2.0 * a);

      return { root, root };
    } else if (discriminant < 0.0) {
      /* No real roots. */

      return {};
    }

    /* Two real roots. */

    const double q = std::sqrt(discriminant);
    const double a2 = 2.0 * a;

    return { (q - b) / a2, (-b - q) / a2 };
  }

  CubicSolutions<double> solve_cubic(const double a, const double b, const double c, const double d) {
    if (is_almost_zero(a)) {
      /* It is a quadratic equation */

      return solve_quadratic(b, c, d);
    }

    if (is_almost_zero(d)) {
      /* One root is 0. */

      CubicSolutions solutions = solve_quadratic(a, b, c);
      solutions.count++;

      return solutions;
    }

    /* Calculate coefficients of the depressed cubic equation: y^3 + py + q = 0 */
    const double p = (3.0 * a * c - b * b) / (3.0 * a * a);
    const double q = (2.0 * b * b * b - 9.0 * a * b * c + 27.0 * a * a * d) / (27.0 * a * a * a);

    /* Calculate discriminant */
    const double discriminant = (q * q) / 4.0 + (p * p * p) / 27.0;

    if (is_almost_zero(discriminant)) {
      const double u = std::cbrt(-q / 2.0);
      /* Three real roots, two of them are equal */
      const double realRoot1 = 2.0 * u - b / (3.0 * a);
      const double realRoot2 = -u - b / (3.0 * a);

      return { realRoot1, realRoot2, realRoot2 };
    } else if (discriminant > 0) {
      const double u = std::cbrt(-q / 2.0 + std::sqrt(discriminant));

      /* One real root and two complex roots */
      const double v = std::cbrt(-q / 2.0 - std::sqrt(discriminant));
      const double realRoot = u + v - b / (3.0 * a);

      return { realRoot };
    } else {
      const double phi = std::acos(-q / 2.0 * std::sqrt(-27.0 / (p * p * p)));
      const double b1 = -b / (3.0 * a);
      const double xi = 2.0 * std::sqrt(-p / 3.0);

      /* Three distinct real roots */
      const double root1 = xi * std::cos(phi / 3.0) + b1;
      const double root2 = xi * std::cos((phi + two_pi<double>) / 3.0) + b1;
      const double root3 = xi * std::cos((phi + 2.0 * two_pi<double>) / 3.0) + b1;

      return { root1, root2, root3 };
    }
  }

}
