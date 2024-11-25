/**
 * @file io/text/codepoint.h
 * @brief The file contains unicode codepoint utilities.
 */

#pragma once

#include <cstdint>

namespace graphick::io::text {

/**
 * @brief Presentation mode for an emoji cluster.
 */
enum class Emoji : uint8_t {
  None = 0,     // Not an emoji.
  Default = 1,  // Default emoji presentation.
  Text = 2,     // Emoji with text presentation.
  Color = 3,    // Emoji with color presentation.
};

/**
 * @brief White space content of a cluster.
 */
enum class Whitespace : uint8_t {
  None = 0,          // Not a space.
  Space = 1,         // Standard space.
  NoBreakSpace = 2,  // Non-breaking space (U+00A0).
  Tab = 3,           // Horizontal tab.
  Newline = 4,       // Newline (CR, LF, or CRLF).
  Other = 5,         // Other space.
};

/**
 * @brief Boundary type of a character or cluster.
 */
enum class Boundary : uint8_t {
  None = 0,       // Not a boundary.
  Word = 1,       // Start of a word.
  Line = 2,       // Potential line break.
  Mandatory = 3,  // Mandatory line break.
};

/**
 * @brief Information about a cluster including content properties and boundary analysis.
 */
struct ClusterInfo {
  uint16_t info;  // The underlying cluster information.

  static constexpr uint16_t BOUND_SHIFT = 14;
  static constexpr uint16_t SPACE_SHIFT = 1;
  static constexpr uint16_t EMOJI_SHIFT = 8;
  static constexpr uint16_t SPACE_MASK = 0b111;
  static constexpr uint16_t EMOJI_MASK = 0b11;

  ClusterInfo() : info(0) {}
  ClusterInfo(uint16_t info) : info(info) {}

  /**
   * @brief Returns whether the cluster is missing an appropriate base character.
   *
   * @return true if the cluster is missing an appropriate base character, false otherwise.
   */
  inline bool is_broken() const
  {
    return info & 1 != 0;
  }

  /**
   * @brief Returns whether the cluster is an emoji.
   *
   * @return true if the cluster is an emoji, false otherwise.
   */
  inline bool is_emoji() const
  {
    return (info >> EMOJI_SHIFT & EMOJI_MASK) != 0;
  }

  /**
   * @brief Returns the emoji presentation mode of the cluster.
   *
   * @return The emoji presentation mode of the cluster.
   */
  inline Emoji emoji() const
  {
    return static_cast<Emoji>(info >> EMOJI_SHIFT & EMOJI_MASK);
  }

  /**
   * @brief Returns whether the cluster is whitespace.
   *
   * @return true if the cluster is whitespace, false otherwise.
   */
  inline bool is_whitespace() const
  {
    return (info >> SPACE_SHIFT & SPACE_MASK) != 0;
  }

  /**
   * @brief Returns the whitespace content of the cluster.
   *
   * @return The whitespace content of the cluster.
   */
  inline Whitespace whitespace() const
  {
    return static_cast<Whitespace>(info >> SPACE_SHIFT & SPACE_MASK);
  }

  /**
   * @brief Returns whether the cluster is a boundary.
   *
   * @return true if the cluster is a boundary, false otherwise.
   */
  inline bool is_boundary() const
  {
    return (info >> BOUND_SHIFT) != 0;
  }

  /**
   * @brief Returns the boundary state of the cluster.
   *
   * @return The boundary state of the cluster.
   */
  inline Boundary boundary() const
  {
    return static_cast<Boundary>(info >> BOUND_SHIFT);
  }

  /**
   * @brief Sets the cluster as broken.
   */
  inline void set_broken()
  {
    info |= 1;
  }

  /**
   * @brief Sets the cluster as an emoji.
   *
   * @param emoji The emoji presentation mode.
   */
  inline void set_emoji(Emoji emoji)
  {
    info = info & ~(EMOJI_MASK << EMOJI_SHIFT) | static_cast<uint16_t>(emoji) << EMOJI_SHIFT;
  }

  /**
   * @brief Sets the cluster as whitespace.
   *
   * @param space The whitespace content.
   */
  inline void set_space(Whitespace space)
  {
    info = info & ~(SPACE_MASK << SPACE_SHIFT) | static_cast<uint16_t>(space) << SPACE_SHIFT;
  }

  /**
   * @brief Sets the appropriate space content from a character.
   *
   * @param ch The character to set the space content from.
   */
  inline void set_space_from_char(char ch)
  {
    switch (ch) {
      case ' ':
        set_space(Whitespace::Space);
        break;
      case '\u00A0':
        set_space(Whitespace::NoBreakSpace);
        break;
      case '\t':
        set_space(Whitespace::Tab);
        break;
      default:
        break;
    }
  }

  /**
   * @brief Merges the boundary state with the cluster.
   *
   * @param boundary The boundary state to merge.
   */
  inline void merge_boundary(uint16_t boundary)
  {
    const uint16_t shifted = info >> BOUND_SHIFT;
    const uint16_t bits = (shifted > boundary ? shifted : boundary) << BOUND_SHIFT;
    info = ((info << 2) >> 2) | bits;
  }
};

/**
 * @brief Character cluster, output from the parser and input to the shaper.
 */
struct CharCluster {
  ClusterInfo info;  // The cluster information.

  CharCluster() : info(0) {}
};

}  // namespace graphick::io::text
