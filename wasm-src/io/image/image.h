/**
 * @file io/image.h
 * @brief The file contains the definition of the image utility functions.
 */

#pragma once

#include "../../math/vec2.h"

namespace graphick::io {

/**
 * @brief The Image struct is a lightweight wrapper around the image data.
 *
 * Image data is generally menaged by the ResourceManager.
 */
struct Image {
  uint8_t* data;     // A pointer to the image data.
  ivec2 size;        // The size of the image.
  uint8_t channels;  // The number of channels of the image.
};

}  // namespace graphick::io
