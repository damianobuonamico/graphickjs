/**
 * @file io/text/unicode.h
 * @brief Unicode character properties.
 */

#pragma once

#include "unicode_data.h"

namespace graphick::io::text {

/**
 * @brief Compact, constant time reference to Unicode properties for a character
 */
struct CharInfo {
  uint16_t data;  // The underlying data.

  inline operator uint16_t() const
  {
    return data;
  }
};

/**
 * @brief Shape class of a character.
 */
enum class ShapeClass {
  Reph,      // Reph form.
  Pref,      // Pre-base form.
  Kinzi,     // Myanmar three character prefix.
  Base,      // Base character.
  Mark,      // Mark character.
  Halant,    // Halant modifier.
  MedialRa,  // Medial consonant Ra.
  VMPre,     // Pre-base vowel modifier.
  VPre,      // Pre-base dependent vowel.
  VBlw,      // Below base dependent vowel.
  Anusvara,  // Anusvara class.
  Zwj,       // Zero width joiner.
  Zwnj,      // Zero width non-joiner.
  Control,   // Control character.
  Vs,        // Variation selector.
  Other      // Other character.
};

}  // namespace graphick::io::text
