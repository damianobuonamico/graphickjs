/**
 * @file io/text.h
 * @brief The file contains the definition of the text utility functions.
 */

#pragma once

namespace graphick::io {

/**
 * @brief The text metrics.
 */
struct TextMetrics {
  float width;    // The width of the text.
  float height;   // The height of the text.

  float ascent;   // The ascent of the text.
  float descent;  // The descent of the text.
};

}  // namespace graphick::io
