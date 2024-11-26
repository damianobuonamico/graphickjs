/**
 * @file io/text/font.cpp
 * @brief The file contains the implementation of the font utility functions.
 *
 * @todo move the font loading here
 */

#include "font.h"

#include "../../lib/stb/stb_truetype.h"

namespace graphick::io::text {

inline static stbtt_fontinfo* to_stbtt_fontinfo(FontInfo* font_info)
{
  return static_cast<stbtt_fontinfo*>(static_cast<void*>(font_info));
}

inline static const stbtt_fontinfo* to_stbtt_fontinfo(const FontInfo* font_info)
{
  return static_cast<const stbtt_fontinfo*>(static_cast<const void*>(font_info));
}

Font::Font(const uint8_t* data, const size_t size)
{
  m_data = new uint8_t[size];
  std::copy(data, data + size, m_data);

  stbtt_fontinfo* font_info = to_stbtt_fontinfo(&m_info);

  if (!stbtt_InitFont(font_info, m_data, 0)) {
    delete[] m_data;
    m_data = nullptr;
    return;
  }

  int line_spacing;
  stbtt_GetFontVMetrics(font_info, nullptr, nullptr, &line_spacing);

  m_scale_factor = stbtt_ScaleForPixelHeight(font_info, 1.0f);
  m_line_spacing = m_scale_factor * line_spacing;
}

float Font::get_kerning(const int glyph1, const int glyph2) const
{
  return stbtt_GetCodepointKernAdvance(to_stbtt_fontinfo(&m_info), glyph1, glyph2) *
         m_scale_factor;
}

const Glyph Font::load_glyph(const int codepoint) const
{
  const stbtt_fontinfo* font_info = to_stbtt_fontinfo(&m_info);

  stbtt_vertex* vertices;
  size_t num_vertices;
  Glyph glyph;

  glyph.index = stbtt_FindGlyphIndex(font_info, codepoint);

  int x0, y0, x1, y1, advance;
  stbtt_GetGlyphBox(font_info, glyph.index, &x0, &y0, &x1, &y1);
  stbtt_GetGlyphHMetrics(font_info, glyph.index, &advance, nullptr);

  glyph.bounding_rect = rect{vec2(x0, -y1) * m_scale_factor, vec2(x1, -y0) * m_scale_factor};
  glyph.advance = advance * m_scale_factor;

  num_vertices = stbtt_GetGlyphShape(font_info, glyph.index, &vertices);

  if (num_vertices == 0) {
    return glyph;
  }

  const vec2 curves_factor = 1.0f / glyph.bounding_rect.size();

  for (int j = 0; j < num_vertices; j++) {
    const stbtt_vertex& vertex = vertices[j];

    switch (vertex.type) {
      case STBTT_vline:
        glyph.path.line_to((m_scale_factor * vec2(vertex.x, -vertex.y) - glyph.bounding_rect.min) *
                           curves_factor);
        break;
      case STBTT_vcurve:
        glyph.path.quadratic_to(
            (m_scale_factor * vec2(vertex.cx, -vertex.cy) - glyph.bounding_rect.min) *
                curves_factor,
            (m_scale_factor * vec2(vertex.x, -vertex.y) - glyph.bounding_rect.min) *
                curves_factor);
        break;
      case STBTT_vcubic:
        glyph.path.cubic_to(
            (m_scale_factor * vec2(vertex.cx, -vertex.cy) - glyph.bounding_rect.min) *
                curves_factor,
            (m_scale_factor * vec2(vertex.cx1, -vertex.cy1) - glyph.bounding_rect.min) *
                curves_factor,
            (m_scale_factor * vec2(vertex.x, -vertex.y) - glyph.bounding_rect.min) *
                curves_factor);
        break;
      default:
      case STBTT_vmove:
        glyph.path.move_to((m_scale_factor * vec2(vertex.x, -vertex.y) - glyph.bounding_rect.min) *
                           curves_factor);
        break;
    }
  }

  delete[] vertices;

  return glyph;
}

}  // namespace graphick::io::text
