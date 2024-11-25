/**
 * @file io/text/shaper.cpp
 * @brief The file contains the implementation of the basic text shaper.
 */

#include "shaper.h"

#include "parser.h"

namespace graphick::io::text {

void Shaper::shape(const std::string& text)
{
  CharCluster cluster;
  Parser().parse(text);
}

}  // namespace graphick::io::text
