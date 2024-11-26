/**
 * @file io/font.h
 * @brief The file contains the definition of the font utility functions.
 */

#pragma once

#include "../../geom/quadratic_path.h"

#include <unordered_map>

namespace graphick::io::text {

/**
 * @brief The glyph data.
 */
struct Glyph {
  int index;                       // The glyph index, use this instead of the codepoint for speed.
  float advance;                   // Offset from the current position to the next.
  rect bounding_rect;              // The bounding rectangle of the glyph.

  geom::quadratic_multipath path;  // The glyph path.
};

/**
 * @brief The font information, compatible with stb_truetype.
 */
struct FontInfo {
  struct FontInfoBuffer {
    unsigned char* data;       // Pointer to the buffer data.
    int cursor;                // The current cursor position.
    int size;                  // The size of the buffer.
  };

  void* userdata;              // user data to be passed to malloc/free.
  unsigned char* data;         // Pointer to .ttf file.
  int fontstart;               // Offset of start of font.

  int num_glyphs;              // Number of glyphs, needed for range checking.

  int loca, head, glyf, hhea, hmtx, kern, gpos,
      svg;                     // Table locations as offset from start of .ttf.
  int index_map;               // A cmap mapping for our chosen character encoding.
  int index_to_loc_format;     // Format needed to map from glyph index to glyph.

  FontInfoBuffer cff;          // CFF font data.
  FontInfoBuffer charstrings;  // The charstring index.
  FontInfoBuffer gsubrs;       // Global charstring subroutines index.
  FontInfoBuffer subrs;        // Private charstring subroutines index.
  FontInfoBuffer fontdicts;    // Array of font dicts.
  FontInfoBuffer fdselect;     // Map from glyph to fontdict.
};

/**
 * @brief The font.
 */
class Font {
 public:
  /**
   * @brief Copies the font file locally. It doesn't delete the original data.
   *
   * A copy is necessary because the original data can be allocated by wasm in a js module.
   *
   * @param data The font file.
   */
  Font(const uint8_t* data, const size_t size);

  /**
   * @brief Copy and move constructors.
   */
  Font(const Font&) = delete;
  Font(Font&& other)
      : m_data(other.m_data),
        m_info(other.m_info),
        m_scale_factor(other.m_scale_factor),
        m_line_spacing(other.m_line_spacing),
        m_glyphs(std::move(other.m_glyphs))
  {
    other.m_data = nullptr;
  }

  /**
   * @brief Assignment operators.
   */
  Font& operator=(const Font&) = delete;
  Font& operator=(Font&& other)
  {
    if (this != &other) {
      delete[] m_data;

      m_data = other.m_data;
      m_info = other.m_info;
      m_scale_factor = other.m_scale_factor;
      m_line_spacing = other.m_line_spacing;
      m_glyphs = std::move(other.m_glyphs);

      other.m_data = nullptr;
    }

    return *this;
  }

  /**
   * @brief Frees the font data.
   */
  ~Font()
  {
    delete[] m_data;
  }

  /**
   * @brief Returns whether the font is valid.
   *
   * @return Whether the font is valid.
   */
  inline bool valid() const
  {
    return m_data != nullptr;
  }

  /**
   * @brief Returns the glyph for the given codepoint.
   *
   * @param codepoint The Unicode codepoint.
   * @return The glyph for the codepoint.
   */
  inline const Glyph& get_glyph(const int codepoint) const
  {
    const auto it = m_glyphs.find(codepoint);

    if (it != m_glyphs.end()) {
      return it->second;
    }

    return m_glyphs.insert(std::make_pair(codepoint, std::move(load_glyph(codepoint))))
        .first->second;
  }

  /**
   * @brief Returns the kerning between two glyphs.
   *
   * @param glyph1 The first glyph index.
   * @param glyph2 The second glyph index.
   * @return The kerning between the two glyphs.
   */
  float get_kerning(const int glyph1, const int glyph2) const;

 private:
  /**
   * @brief Loads the glyph for the given codepoint.
   *
   * @param codepoint The Unicode codepoint.
   * @return The glyph for the codepoint.
   */
  const Glyph load_glyph(const int codepoint) const;

 private:
  uint8_t* m_data;       // The font data.
  FontInfo m_info;       // The font information, (specific to stb_truetype).

  float m_scale_factor;  // Factor to multiply the glyphs by to get unit scale.
  float m_line_spacing;  // The distance between two lines of text, in unit scale.

  mutable std::unordered_map<int, Glyph> m_glyphs;  // A map of cached codepoints to glyphs.
};

}  // namespace graphick::io::text
