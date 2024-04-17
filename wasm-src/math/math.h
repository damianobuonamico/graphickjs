/**
 * @file math/math.h
 * @brief Contains various math functions and structs used throughout the Graphick editor.
 *
 * @todo implement solve_cubic_approx(), solve_quartic(), solve_quintic()
 */

#pragma once

#include "scalar.h"
#include "vector.h"
#include "rect.h"

#include <optional>
#include <vector>
#include <tuple>

namespace graphick::math {

  /**
   * @brief Struct containing solutions to a quadratic equation.
   *
   * @struct QuadraticSolutions
   */
  template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
  struct QuadraticSolutions {
    T solutions[2];    /* The solutions. */
    uint8_t count;     /* The number of solutions. */

    /**
     * @brief Constructs a QuadraticSolutions object.
     */
    QuadraticSolutions() : count(0), solutions{ T(0), T(0) } {}
    QuadraticSolutions(const T x) : count(1), solutions{ x, T(0) } {}
    QuadraticSolutions(const T x1, const T x2) : count(2), solutions{ x1, x2 } {}

    template <typename U>
    QuadraticSolutions(const QuadraticSolutions<U>& quadratic) :
      count(quadratic.count),
      solutions{ static_cast<T>(quadratic.solutions[0]), static_cast<T>(quadratic.solutions[1]) } {}
  };

  /**
   * @brief Struct containing solutions to a cubic equation.
   *
   * @struct CubicSolutions
   */
  template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
  struct CubicSolutions {
    T solutions[3];    /* The solutions. */
    uint8_t count;          /* The number of solutions. */

    /**
     * @brief Constructs a CubicSolutions object.
     */
    CubicSolutions() : count(0), solutions{ T(0), T(0), T(0) } {}
    CubicSolutions(const T x) : count(1), solutions{ x, T(0), T(0) } {}
    CubicSolutions(const T x1, const T x2) : count(2), solutions{ x1, x2, T(0) } {}
    CubicSolutions(const T x1, const T x2, const T x3) : count(3), solutions{ x1, x2, x3 } {}
    CubicSolutions(const QuadraticSolutions<T>& quadratic) : count(quadratic.count), solutions{ quadratic.solutions[0], quadratic.solutions[1], T(0) } {}

    template <typename U>
    CubicSolutions(const CubicSolutions<U>& cubic) :
      count(cubic.count),
      solutions{ static_cast<T>(cubic.solutions[0]), static_cast<T>(cubic.solutions[1]), static_cast<T>(cubic.solutions[2]) } {}
  };

  /**
   * @brief Solves a linear equation of the form ax + b = 0.
   *
   * Assumes that a != 0.
   *
   * @param a The coefficient of x.
   * @param b The constant term.
   * @return The solution for x.
   */
  template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
  inline T solve_linear(const T a, const T b) {
    return -b / a;
  }

  /**
   * @brief Solves a quadratic equation of the form ax^2 + bx + c = 0.
   *
   * @param a The coefficient of x^2.
   * @param b The coefficient of x.
   * @param c The constant term.
   * @return A struct containing the real solutions to the equation.
   */
  QuadraticSolutions<double> solve_quadratic(const double a, const double b, const double c);

  /**
   * @brief Solves a quadratic equation of the form ax^2 + bx + c = 0.
   *
   * @param a The coefficient of x^2.
   * @param b The coefficient of x.
   * @param c The constant term.
   * @return A struct containing the real solutions to the equation.
   */
  inline QuadraticSolutions<float> solve_quadratic(const float a, const float b, const float c) {
    return solve_quadratic(static_cast<double>(a), static_cast<double>(b), static_cast<double>(c));
  }

  /**
   * @brief Solves a cubic equation of the form ax^3 + bx^2 + cx + d = 0.
   *
   * @param a The coefficient of x^3.
   * @param b The coefficient of x^2.
   * @param c The coefficient of x.
   * @param d The constant term.
   * @return A struct containing the real solutions of the cubic equation.
   */
  CubicSolutions<double> solve_cubic(const double a, const double b, const double c, const double d);

  /**
   * @brief Solves a cubic equation of the form ax^3 + bx^2 + cx + d = 0.
   *
   * @param a The coefficient of x^3.
   * @param b The coefficient of x^2.
   * @param c The coefficient of x.
   * @param d The constant term.
   * @return A struct containing the real solutions of the cubic equation.
   */
  inline CubicSolutions<float> solve_cubic(const float a, const float b, const float c, const float d) {
    return solve_cubic(static_cast<double>(a), static_cast<double>(b), static_cast<double>(c), static_cast<double>(d));
  }

  /**
   * @brief Calculates a hash value for a list of numbers.
   *
   * @param numbers The list of numbers.
   * @return The hash value.
   */
  template <typename T>
  inline int hash(std::initializer_list<T> numbers) {
    int h = 1;

    for (T n : numbers) {
      int i = *(int*)(&n);
      h = 31 * h + i;
    }

    h ^= (h >> 20) ^ (h >> 12);
    return h ^ (h >> 7) ^ (h >> 4);
  }

}
