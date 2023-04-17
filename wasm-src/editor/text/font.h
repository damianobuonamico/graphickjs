#pragma once

#include <hb.h>
// #include <ft2build.h>
// #include FT_FREETYPE_H
// #define HZ_IMPLEMENTATION
// #include "../../utils/hamza/hz.h"

#include "../../utils/hamza/stb_truetype.h"

#include <stddef.h>
#include <stdint.h>

class Font {
public:
  struct FontData {
    // hz_shaper_t* shaper;
    // hz_font_data_t* font_data;
    stbtt_fontinfo* font_info;
  };
public:
  Font(const uint8_t* buffer, size_t buffer_size);
  ~Font();

  inline FontData get() { return { &m_font_info }; }
private:
  // FT_Library m_library;
  // FT_Face m_ft_face = nullptr;

  uint8_t* m_buffer = nullptr;
  size_t m_buffer_size;

  stbtt_fontinfo m_font_info;

  // stbtt_fontinfo m_font_info;
  // hz_font_data_t m_font_data;
  // hz_shaper_t m_shaper;
  // hz_font_t* m_font = nullptr;

  hb_blob_t* m_blob = nullptr;
  hb_face_t* m_face = nullptr;
  hb_font_t* m_font = nullptr;
};
