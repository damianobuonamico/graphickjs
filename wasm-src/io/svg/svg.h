/**
 * @file svg.h
 * @brief Contains the declaration of the SVG parser.
 *
 * @todo decide whether to return EncodedData (or similar) or add directly to the scene
 * @todo implement SVG encoding
 */

#pragma once

#include <string>

namespace Graphick::io::svg {

  /**
   * @brief Parse an SVG string and add the elements to the scene.
   *
   * @param svg The SVG string to parse.
   * @return true if the SVG was parsed successfully, false otherwise.
   */
  bool parse_svg(const std::string& svg);

  /**
   * @brief Parse an SVG string and add the elements to the scene.
   *
   * @param svg The SVG string to parse.
   * @return true if the SVG was parsed successfully, false otherwise.
   */
  bool parse_svg(const char* svg);

}
