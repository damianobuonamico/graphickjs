/**
 * @file io/text/shaper.h
 * @brief The file contains a basic text shaper.
 */

#pragma once

#include <string>

namespace graphick::io::text {

/**
 * @brief The Shaper class is a basic text shaper (latin only) inspired by
 * <https://github.com/dfrg/swash>
 */
class Shaper {
 public:
  void shape(const std::string& text);

 private:
};

};  // namespace graphick::io::text
