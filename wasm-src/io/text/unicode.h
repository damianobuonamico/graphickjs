/**
 * @file io/text/unicode.h
 * @brief Unicode UTF-8 encoding and decoding functions.
 */

#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace graphick::io::text {

/**
 * @brief Information about the encoding of a UTF-8 character.
 */
struct utf8_encoding {
  uint8_t mask;
  uint8_t value;
  uint8_t extra;
};

constexpr utf8_encoding utf8_info[4] = {
    {0x80, 0x00, 0}, {0xE0, 0xC0, 1}, {0xF0, 0xE0, 2}, {0xF8, 0xF0, 3}};

/**
 * @brief Decodes a single UTF-8 codepoint from a stream.
 *
 * @param stream The stream to read from.
 * @return The Unicode codepoint, or -1 if the stream is invalid.
 */
inline int utf8_codepoint(std::istream& stream)
{
  uint8_t next = stream.get();

  if (next == EOF) {
    return -1;
  }

  for (const auto& type : utf8_info) {
    if (static_cast<uint8_t>(next & type.mask) == type.value) {
      int result = next & ~type.mask;
      int count = type.extra;

      for (; count; --count) {
        const uint8_t next = stream.get();

        if ((next & 0xC0) != 0x80) {
          /* Not a valid continuation character. */
          stream.setstate(std::ios::badbit);
          return -1;
        }

        result = (result << 6) | (next & 0x3F);
      }

      return result;
    }
  }

  /* Not a valid first character. */
  stream.setstate(std::ios::badbit);
  return -1;
}

/**
 * @brief Decodes a UTF-8 string into a vector of codepoints.
 *
 * @param utf8 The UTF-8 string to decode.
 * @return The vector of Unicode codepoints.
 */
inline std::vector<int> utf8_decode(const std::string& utf8)
{
  std::istringstream stream(utf8);
  std::vector<int> codepoints;

  while (stream.good()) {
    int codepoint = utf8_codepoint(stream);

    if (codepoint < 0) {
      break;
    }

    codepoints.push_back(codepoint);
  }

  return codepoints;
}

}  // namespace graphick::io::text
