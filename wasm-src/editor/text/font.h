// #pragma once

// #include "../../io/ttf/stb_truetype.h"
// #include "../../renderer/geometry/geometry.h"

// #include <hb.h>

// #include <stddef.h>
// #include <stdint.h>
// #include <unordered_map>

// class Font {
// public:
//   struct FontData {
//     hb_font_t* font;
//     stbtt_fontinfo* font_info;
//   };
// public:
//   Font(const uint8_t* buffer, size_t buffer_size);
//   ~Font();

//   inline FontData get() { return { m_font, &m_font_info }; }

//   const Geometry& request_glyph(hb_codepoint_t glyph_id, float scale);
// private:
//   uint8_t* m_buffer = nullptr;
//   size_t m_buffer_size;

//   stbtt_fontinfo m_font_info;

//   hb_blob_t* m_blob = nullptr;
//   hb_face_t* m_face = nullptr;
//   hb_font_t* m_font = nullptr;

//   std::unordered_map<hb_codepoint_t, Geometry> m_glyphs;
// };
