#pragma once

#include <string>

namespace Graphick::io::svg {

  /**
   * TODO: Document
   * TOOD: Decide weather to return a tree or add directly to the scene
   */
  bool parse_svg(const std::string& svg);
  bool parse_svg(const char* svg);

}
