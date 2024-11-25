/**
 * @file io/font.h
 * @brief The file contains the definition of the font utility functions.
 */

#pragma once

#include "../../geom/quadratic_path.h"

#include <unordered_map>

namespace graphick::io {

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
 *
 * @todo convert to class with private members and public getters
 */
struct Font {
  uint8_t* data;       // The font data.
  FontInfo info;       // The font information, (specific to stb_truetype).

  float scale_factor;  // Factor to multiply the glyphs by to get unit scale.
  float ascent;        // The ascent of the font, in unit scale.
  float descent;       // The descent of the font, in unit scale.
  float line_spacing;  // The distance between two lines of text, in unit scale.

  std::unordered_map<uint32_t, geom::quadratic_multipath> glyphs;  // The loaded glyphs map.

  /**
   * @brief Copies the font file locally. It doesn't delete the original data.
   *
   * A copy is necessary because the original data can be allocated by wasm in a js module.
   *
   * @param data The font file.
   */
  inline Font(const uint8_t* data, const size_t size)
  {
    this->data = new uint8_t[size];
    std::copy(data, data + size, this->data);
  }

  /**
   * @brief Copy and move constructors.
   */
  Font(const Font&) = delete;
  Font(Font&& other)
      : data(other.data),
        info(other.info),
        scale_factor(other.scale_factor),
        ascent(other.ascent),
        descent(other.descent),
        line_spacing(other.line_spacing),
        glyphs(std::move(other.glyphs))
  {
    other.data = nullptr;
  }

  /**
   * @brief Assignment operators.
   */
  Font& operator=(const Font&) = delete;
  Font& operator=(Font&& other)
  {
    if (this != &other) {
      delete[] data;

      data = other.data;
      info = other.info;
      scale_factor = other.scale_factor;
      ascent = other.ascent;
      descent = other.descent;
      line_spacing = other.line_spacing;
      glyphs = std::move(other.glyphs);

      other.data = nullptr;
    }

    return *this;
  }

  /**
   * @brief Frees the font data.
   */
  inline ~Font()
  {
    delete[] data;
  }
};

}  // namespace graphick::io
