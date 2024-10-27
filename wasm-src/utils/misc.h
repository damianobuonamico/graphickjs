/**
 * @file misc.h
 * @brief The file contains the definition of the miscellaneous utilities.
 */

#pragma once

#include <initializer_list>

namespace graphick::utils {

/**
 * @brief Iterates over two containters and applies a function to each element.
 *
 * @param first The first container.
 * @param second The second container.
 * @param func The function to apply to each element.
 * @return The function.
 */
template <typename T, typename Fn>
const Fn for_each(const T& first, const T& second, Fn func) {
  for (auto it = first.begin(); it != first.end(); it++) {
    func(*it);
  }
  for (auto it = second.begin(); it != second.end(); it++) {
    func(*it);
  }

  return func;
}

}  // namespace graphick::utils
