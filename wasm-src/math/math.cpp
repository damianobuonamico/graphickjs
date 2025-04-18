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

QuadraticSolutions<double> solve_quadratic(const double a, const double b, const double c)
{
  const double sc = c / a;
  const double sb = b / a;

  if (!(std::isfinite(sc) && std::isfinite(sb))) {
    const double root = -c / b;
    if (std::isfinite(root)) {
      return {root};
    } else if (c == 0.0 && b == 0.0) {
      return {0.0};
    } else {
      return {};
    }
  }

  const double det = sb * sb - 4.0 * sc;
  double root1 = 0.0;

  if (std::isfinite(det)) {
    if (det < 0.0) {
      return {};
    } else if (det == 0) {
      return {-0.5 * sb};
    }
    root1 = -0.5 * (sb + std::copysign(std::sqrt(det), sb));
  } else {
    root1 = -sb;
  }

  const double root2 = sc / root1;

  if (std::isfinite(root2)) {
    if (root2 > root1) {
      return {root1, root2};
    } else {
      return {root2, root1};
    }
  }

  return {root1};
}

QuadraticSolutions<double> solve_quadratic_normalized(const double a,
                                                      const double b,
                                                      const double c)
{
  const double sc = c / a;
  const double sb = b / a;

  if (!(std::isfinite(sc) && std::isfinite(sb))) {
    const double root = -c / b;
    if (std::isfinite(root) && is_normalized(root)) {
      return {root};
    } else if (c == 0.0 && b == 0.0) {
      return {0.0};
    } else {
      return {};
    }
  }

  const double det = sb * sb - 4.0 * sc;
  double root1 = 0.0;

  if (std::isfinite(det)) {
    if (det < 0.0) {
      return {};
    } else if (det == 0) {
      return {-0.5 * sb};
    }
    root1 = -0.5 * (sb + std::copysign(std::sqrt(det), sb));
  } else {
    root1 = -sb;
  }

  const double root2 = sc / root1;

  const bool root1_norm = is_normalized(root1);
  const bool root2_norm = is_normalized(root2);

  if (std::isfinite(root2) && root2_norm) {
    if (!root1_norm) {
      return {root2};
    }

    if (root2 > root1) {
      return {root1, root2};
    } else {
      return {root2, root1};
    }
  }

  if (root1_norm) {
    return {root1};
  } else {
    return {};
  }
}

CubicSolutions<double> solve_cubic(const double a,
                                   const double b,
                                   const double c,
                                   const double d,
                                   const bool include_double_roots)
{
  // NOTE: if needed, there is a more stable imlpementation in kurbo

  if (is_almost_zero(a)) {
    /* It is a quadratic equation */

    return solve_quadratic(b, c, d);
  }

  if (is_almost_zero(d)) {
    /* One root is 0. */

    CubicSolutions solutions = solve_quadratic(a, b, c);

    if (!include_double_roots) {
      if ((solutions.count == 0) || (solutions.count == 1 && solutions.solutions[0] != 0) ||
          (solutions.count == 2 && solutions.solutions[0] != 0 && solutions.solutions[1] != 0))
      {
        solutions.count++;
      }
    } else {
      solutions.count++;
    }

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
    const double real_root1 = 2.0 * u - b / (3.0 * a);
    const double real_root2 = -u - b / (3.0 * a);

    if (include_double_roots)
      return {real_root1, real_root2, real_root2};

    return {real_root1, real_root2};
  } else if (discriminant > 0) {
    const double u = std::cbrt(-q / 2.0 + std::sqrt(discriminant));

    /* One real root and two complex roots */
    const double v = std::cbrt(-q / 2.0 - std::sqrt(discriminant));
    const double real_root = u + v - b / (3.0 * a);

    return {real_root};
  } else {
    const double phi = std::acos(-q / 2.0 * std::sqrt(-27.0 / (p * p * p)));
    const double b1 = -b / (3.0 * a);
    const double xi = 2.0 * std::sqrt(-p / 3.0);

    /* Three distinct real roots */
    const double root1 = xi * std::cos(phi / 3.0) + b1;
    const double root2 = xi * std::cos((phi + two_pi<double>) / 3.0) + b1;
    const double root3 = xi * std::cos((phi + 2.0 * two_pi<double>) / 3.0) + b1;

    return {root1, root2, root3};
  }
}

CubicSolutions<double> solve_cubic_normalized(const double a,
                                              const double b,
                                              const double c,
                                              const double d,
                                              const bool include_double_roots)
{
  if (is_almost_zero(a)) {
    /* It is a quadratic equation */

    return solve_quadratic_normalized(b, c, d);
  }

  if (is_almost_zero(d)) {
    /* One root is 0. */

    CubicSolutions solutions = solve_quadratic_normalized(a, b, c);

    if (!include_double_roots) {
      if ((solutions.count == 0) || (solutions.count == 1 && solutions.solutions[0] != 0) ||
          (solutions.count == 2 && solutions.solutions[0] != 0 && solutions.solutions[1] != 0))
      {
        solutions.count++;
      }
    } else {
      solutions.count++;
    }

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
    const double real_root1 = 2.0 * u - b / (3.0 * a);
    const double real_root2 = -u - b / (3.0 * a);

    const bool real_root1_norm = math::is_normalized(real_root1);
    const bool real_root2_norm = math::is_normalized(real_root2);

    if (real_root1_norm && real_root2_norm) {
      if (include_double_roots)
        return {real_root1, real_root2, real_root2};
      return {real_root1, real_root2};
    } else if (real_root1_norm) {
      return {real_root1};
    } else if (real_root2_norm) {
      return {real_root2, real_root2};
    }

    return {};
  } else if (discriminant > 0) {
    const double u = std::cbrt(-q / 2.0 + std::sqrt(discriminant));

    /* One real root and two complex roots */
    const double v = std::cbrt(-q / 2.0 - std::sqrt(discriminant));
    const double real_root = u + v - b / (3.0 * a);

    if (math::is_normalized(real_root))
      return {real_root};

    return {};
  } else {
    const double phi = std::acos(-q / 2.0 * std::sqrt(-27.0 / (p * p * p)));
    const double b1 = -b / (3.0 * a);
    const double xi = 2.0 * std::sqrt(-p / 3.0);

    /* Three distinct real roots */
    const double root1 = xi * std::cos(phi / 3.0) + b1;
    const double root2 = xi * std::cos((phi + two_pi<double>) / 3.0) + b1;
    const double root3 = xi * std::cos((phi + 2.0 * two_pi<double>) / 3.0) + b1;

    const bool root1_norm = math::is_normalized(root1);
    const bool root2_norm = math::is_normalized(root2);
    const bool root3_norm = math::is_normalized(root3);

    const uint8_t flag = (static_cast<uint8_t>(root1_norm)) |
                         (static_cast<uint8_t>(root2_norm) << 1) |
                         (static_cast<uint8_t>(root3_norm) << 2);

    switch (flag) {
      case 0b001:
        return {root1};
      case 0b010:
        return {root2};
      case 0b011:
        return {root1, root2};
      case 0b100:
        return {root3};
      case 0b101:
        return {root1, root3};
      case 0b110:
        return {root2, root3};
      case 0b111:
        return {root1, root2, root3};
      default:
      case 0b000:
        return {};
    }
  }
}
}  // namespace graphick::math
