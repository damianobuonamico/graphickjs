#include "text.h"

#include "font_manager.h"
#include "../../utils/console.h"
#include "../../math/vector.h"
#include "../../math/math.h"
#include "earcut.h"
#include <array>

// #include "../../utils/libtess/tesselator.h"

Text::Text(const std::string& text, const std::string& font): m_text(text), m_font(font) {
  // hz_buffer_init(&m_buffer);

  // m_buffer = hb_buffer_create();
  // hb_buffer_add_utf8(m_buffer, text.c_str(), (int)text.size(), 0, (int)text.size());

  // hb_buffer_set_direction(m_buffer, HB_DIRECTION_LTR);
  // hb_buffer_set_script(m_buffer, HB_SCRIPT_LATIN);
  // hb_buffer_set_language(m_buffer, hb_language_from_string("en", -1));

  shape();
}

Text::~Text() {
  // hb_buffer_destroy(m_buffer);
  // hz_buffer_release(&m_buffer);
}

bool Text::shape() const {
  std::weak_ptr<Font> font = FontManager::get_font(m_font);
  if (font.expired()) return false;

  // Font::FontData shaper = font.lock()->get();

  // hz_shape_sz1(shaper.shaper, shaper.font_data, HZ_ENCODING_UTF8, m_text.c_str(), &m_buffer);
  // hb_shape(font.lock()->get().font, m_buffer, nullptr, 0);
  m_shaped = true;

  return true;
}

// static void* stdAlloc(void* userData, unsigned int size)
// {
//   int* allocated = (int*)userData;
//   TESS_NOTUSED(userData);
//   *allocated += (int)size;
//   return malloc(size);
// }

// static void stdFree(void* userData, void* ptr)
// {
//   TESS_NOTUSED(userData);
//   free(ptr);
// }

struct Encoding {
  uint8_t mask;
  uint8_t value;
  int extra;
};

Encoding const utf8Info[4] = {
  {0x80, 0x00, 0},
  {0xE0, 0xC0, 1},
  {0xF0, 0xE0, 2},
  {0xF8, 0xF0, 3}
};

int decodeUtf(std::istream& stream, int result, int count) {
  for (; count; --count) {
    uint8_t next = stream.get();
    if ((next & 0xC0) != 0x80) {
      // Not a valid continuation character
      stream.setstate(std::ios::badbit);
      return -1;
    }
    result = (result << 6) | (next & 0x3F);
  }
  return result;
}

int getCodePoint(std::istream& stream) {
  uint8_t next = stream.get();
  if (next == EOF) {
    return -1;
  }
  for (auto const& type : utf8Info) {
    if ((uint8_t)(next & type.mask) == type.value) {
      return decodeUtf(stream, next & ~type.mask, type.extra);
    }
  }
  // Not a valid first character
  stream.setstate(std::ios::badbit);
  return -1;
}

Geometry Text::geometry() const {
  Geometry geo;
  // Geometry geo(GL_TRIANGLE_STRIP);

  if (!m_shaped) {
    if (!shape()) return geo;
  }

  std::weak_ptr<Font> font = FontManager::get_font(m_font);
  if (font.expired()) return false;

  stbtt_fontinfo* font_info = font.lock()->get().font_info;
  float scale = stbtt_ScaleForPixelHeight(font_info, 64);

  int x = 0;

  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(font_info, &ascent, &descent, &lineGap);

  ascent = std::roundf(ascent * scale);
  descent = std::roundf(descent * scale);

  std::vector<int> codepoints;

  std::istringstream stream(m_text);

  while (stream.good()) {
    int codepoint = getCodePoint(stream);
    if (codepoint < 0) break;

    codepoints.push_back(codepoint);
  }

  // for (int i = 0; i < m_text.size(); i++) {
  //   uint8_t ch = m_text[i];
  //   console::bitset(ch);
  //   for (const Encoding& type : utf8Info) {
  //     for (int count = type.extra; count > 0; --count) {
  //       if (i + count >= m_text.size()) {
  //         // Not enough characters left
  //       }

  //       uint8_t next = m_text[i + count];
  //       if (next & 0xC0 != 0x80) {
  //         // Not a valid continuation character
  //       }
  //       result = (result << 6) | (next & 0x3F);
  //     }
  //     return result;
  //   }
  // }

  for (int i = 0; i < codepoints.size(); i++) {
    /* how wide is this character */
    int ax;
    int lsb;
    int index = stbtt_FindGlyphIndex(font_info, codepoints[i]);
    stbtt_GetGlyphHMetrics(font_info, index, &ax, &lsb);
    // stbtt_GetCodepointHMetrics(font_info, m_text[i], &ax, &lsb);
    /* (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character m_text[i].) */

    /* get bounding box for character (may be offset to account for chars that dip above or below the line) */
    // int c_x1, c_y1, c_x2, c_y2;
    // stbtt_GetCodepointBitmapBox(font_info, m_text[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

    /* compute y (different characters have different heights) */
    // int y = ascent + c_y1;

    /* render character (stride and offset is important here) */
    // int byteOffset = x + roundf(lsb * scale) + (y * b_w);
    // stbtt_MakeCodepointBitmap(font_info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, m_text[i]);

    // Box box = { {(float)(x + c_x1), (float)(c_y1)}, {(float)(x + c_x2), (float)(c_y2)} };
    //geo.push_quad(box, vec4{ 1.0f, 1.0f, 1.0f, 1.0f });

    stbtt_vertex* vertices;
    int vertices_len = stbtt_GetGlyphShape(font_info, index, &vertices);
    // int vertices_len = stbtt_GetCodepointShape(font_info, m_text[i], &vertices);

    using Point = std::array<float, 2>;
    std::vector<std::vector<Point>> polygon;

    // uint32_t offset = geo.offset();

    // int allocated = 0;
    // TESSalloc ma;

    // memset(&ma, 0, sizeof(ma));
    // ma.memalloc = stdAlloc;
    // ma.memfree = stdFree;
    // ma.userData = (void*)&allocated;
    // ma.extraVertices = 256; // realloc not provided, allow 256 extra vertices.

    // TESStesselator* tess = tessNewTess(&ma);
    // if (!tess) return {};

    // tessSetOption(tess, TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 1);

    // GLUtesselator* tess = gluNewTess();
    // if (!tess) return {};

    // gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
    // gluTessCallback(tess, GLU_TESS_VERTEX, (GLvoid(*)()) & vertexCallback);

    // gluTessBeginPolygon(tess, nullptr);
    // gluTessBeginContour(tess);

    // std::vector<std::vector<float>> contours;
    //contours.push_back({});
    size_t contour_index = 0;
    uint32_t offset = geo.offset();

    for (int j = 0; j < vertices_len; j++) {
      auto& vertex = vertices[j];
      if (vertex.type == STBTT_vmove) {
        polygon.push_back({});
        // contours.push_back({});
        contour_index = polygon.size() - 1;
        // gluTessEndContour(tess);
        // gluTessBeginContour(tess);
      }

      polygon[contour_index].push_back({ { x + (float)vertex.x * scale, -(float)vertex.y * scale } });
      geo.push_vertex({ vec2{ x + (float)vertex.x * scale, -(float)vertex.y * scale } });

      // contours[contour_index].push_back(x + (float)vertex.x * scale);
      // contours[contour_index].push_back(-(float)vertex.y * scale);

      // geo.push_quad(vec2{ x + (float)vertex.x * scale, -(float)vertex.y * scale }, 1.0f, vec4{ 0.2f, 0.8f, 0.8f, 0.5f });


      // GLdouble v[3] = { vertex.x, vertex.y, 0.0 };

      // gluTessVertex(tess, v, nullptr);
      //     geo.push_vertex({ vec2{ x + (float)vertices[j].x * scale, -(float)vertices[j].y * scale} });
      //   }

      //   for (int j = 2; j < vertices_len; j++) {
      //     geo.push_indices({ offset + j - 2, offset + j - 1, offset + j });
      //   }
    }

    std::vector<bool> windings(polygon.size());
    std::vector<Box> boxes(polygon.size());
    for (int h = 0; h < polygon.size(); h++) {
      float winding = 0;
      Box box = { std::numeric_limits<vec2>::max(), std::numeric_limits<vec2>::min() };
      for (int n = 0; n < polygon[h].size() - 1; n++) {
        winding += (polygon[h][n + 1][0] - polygon[h][n][0]) * (polygon[h][n + 1][1] + polygon[h][n][1]);
      }

      winding += (polygon[h][0][0] - polygon[h][polygon[h].size() - 1][0]) * (polygon[h][0][1] + polygon[h][polygon[h].size() - 1][1]);

      for (int n = 0; n < polygon[h].size(); n++) {
        min(box.min, vec2{ polygon[h][n][0], polygon[h][n][1] }, box.min);
        max(box.max, vec2{ polygon[h][n][0], polygon[h][n][1] }, box.max);
      }

      windings[h] = winding >= 0;

      boxes[h] = box;
    }

    std::vector<std::vector<std::vector<Point>>> polygons;

    int offs = 0;
    //int last_winding = 0;
    for (int h = 1; h < polygon.size(); h++) {
      if (windings[offs] == windings[h] || !is_box_in_box(boxes[h], boxes[offs])) {
        polygons.push_back({});
        polygons.back().insert(polygons.back().end(), polygon.begin() + offs, polygon.begin() + h);
        offs = h;
        // last_winding = h;
      }

      // if (windings[h - 1] == false && windings[h] == false) {
      //   polygons.push_back({});
      //   polygons.back().insert(polygons.back().end(), polygon.begin() + offs, polygon.begin() + h);
      //   offs = h;
      // }
    }

    if (polygon.size() > 1) {
      for (auto& p : polygon[1]) {
        geo.push_quad(vec2{ p[0], p[1] }, 1.0f, vec4{ 0.8f, 0.8f, 0.2f, 0.5f });
      }
    }

    if (offs == 0) {
      std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);

      for (auto& index : indices) {
        geo.push_index(offset + index);
      }
    } else {
      polygons.push_back({});
      polygons.back().insert(polygons.back().end(), polygon.begin() + offs, polygon.end());

      if (polygons.size() > 1) {
        for (auto& p : polygons[1][0]) {
          geo.push_quad(vec2{ p[0], p[1] }, 0.5f, vec4{ 0.8f, 0.2f, 0.2f, 0.5f });
        }
      }

      uint32_t oo = offset;
      for (auto& poly : polygons) {
        std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(poly);

        for (auto& index : indices) {
          geo.push_index(oo + index);
        }

        for (auto& contour : poly) {
          oo += contour.size();
        }
      }
    }


    // gluTessEndContour(tess);
    // gluTessEndPolygon(tess);


    // gluDeleteTess(tess);

    /* advance x */
    x += roundf(ax * scale);

    /* add kerning */
    if (i < codepoints.size() - 1) {
      //int kern = stbtt_GetGlyphKernAdvance(font_info, index, codepoints[i + 1]);
      //x += roundf(kern * scale);
    }

    stbtt_FreeShape(font_info, vertices);

    // for (auto& contour : contours) {
    //   for (int h = 0; h < contour.size(); h += 2) {
    //     // geo.push_quad(vec2{ contour[h], contour[h + 1] }, 1.0f, vec4{ 0.8f, 0.8f, 0.2f, 0.5f });
    //   }
    //   tessAddContour(tess, 2, contour.data(), sizeof(float) * 2, contour.size() / 2);
    // }

    // tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, 3, 2, 0);

    // const float* verts = tessGetVertices(tess);
    // const int* vinds = tessGetVertexIndices(tess);
    // const int nverts = tessGetVertexCount(tess);
    // const int* elems = tessGetElements(tess);
    // const int nelems = tessGetElementCount(tess);

    // for (int h = 0; h < nverts; ++h) {
    //   // geo.push_quad(vec2{ verts[h * 2], verts[h * 2 + 1] }, 1.0f, vec4{ 0.8f, 0.2f, 0.8f, 0.5f });
    //   geo.push_index(geo.offset());
    //   geo.push_vertex({ vec2{ verts[h * 2], verts[h * 2 + 1] }, vec4{1.0f, 1.0f, 1.0f, 0.5f} });
    // }

    // for (int h = 0; h < nelems; ++h) {
    //   const int* p = &elems[h * 2];
    //   for (int j = 0; j < 2 && p[j] != TESS_UNDEF; ++j) {
    //     geo.push_quad(vec2{ verts[p[j] * 2], verts[p[j] * 2 + 1] }, 0.5f, vec4{ 0.8f, 0.2f, 0.8f, 0.5f });

        // geo.push_index(geo.offset());
        // geo.push_vertex({ vec2{ verts[p[j] * 2], verts[p[j] * 2 + 1] }, vec4{1.0f, 1.0f, 1.0f, 0.5f} });
      // }
    // }

    // tessDeleteTess(tess);
  }

  // for (uint32_t i = 2; i < geo.vertices().size(); i++) {
  //   geo.push_indices({ i - 2, i - 1, i });
  // }

  // int pen_x = 0, pen_y = 0;
  // for (size_t i = 0; i < m_buffer.glyph_count; ++i) {
  //   int16_t glyph_index = m_buffer.glyph_indices[i];
  //   hz_glyph_metrics_t* glyph_metrics = &m_buffer.glyph_metrics[i];
  //   int32_t xAdvance = glyph_metrics->xAdvance;
  //   int32_t yAdvance = glyph_metrics->yAdvance;
  //   int32_t xOffset = glyph_metrics->xOffset;
  //   int32_t yOffset = glyph_metrics->yOffset;

  //   int ix0, iy0, ix1, iy1;
  //   stbtt_GetGlyphBitmapBox(font_info, glyph_index, scale, scale, &ix0, &iy0, &ix1, &iy1);

  //   float bx0 = pen_x + ix0;//+ pen_x + roundf(xOffset * scale);
  //   float bx1 = pen_x + ix1;//+ pen_x + roundf(xOffset * scale);
  //   float by0 = pen_y + iy0;//+ pen_y + roundf(yOffset * scale);
  //   float by1 = pen_y + iy1;//+ pen_y + roundf(yOffset * scale);

  //   float xx = roundf(xOffset * scale);
  //   float yy = roundf(yOffset * scale);

  //   // Box box = { std::numeric_limits<vec2>::max(), std::numeric_limits<vec2>::min() };

  //   // xmin = fminf(xmin, bx0 + xx);
  //   // ymin = fminf(ymin, by0 + yy);
  //   // xmax = fmaxf(xmax, bx1 + xx);
  //   // ymax = fmaxf(ymax, by1 + yy);

  //   Box box = { {bx0 + xx, by0 + yy}, {bx1 + xx,by1 + yy} };
  //   geo.push_quad(box, vec4{ 1.0f, 1.0f, 1.0f, 1.0f });

  //   pen_x += roundf(xAdvance * scale);
  // }

  // FT_Face face = font.lock()->get().face;

  // unsigned int glyph_count;
  // hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(m_buffer, &glyph_count);
  // hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(m_buffer, &glyph_count);

  // int mult = 64;

  // hb_position_t cursor_x = 0;
  // hb_position_t cursor_y = 0;
  // for (unsigned int i = 0; i < glyph_count; i++) {
  //   hb_codepoint_t glyphid = glyph_info[i].codepoint;

  //   FT_Load_Glyph(face, glyphid, FT_LOAD_COMPUTE_METRICS);
  //   FT_GlyphSlot glyph = face->glyph;

  //   float xa = (float)glyph_pos[i].x_advance / mult;
  //   float ya = (float)glyph_pos[i].y_advance / mult;
  //   float xo = (float)glyph_pos[i].x_offset / mult;
  //   float yo = (float)glyph_pos[i].y_offset / mult;
  //   float x0 = cursor_x + xo + glyph->metrics.horiBearingX / mult;
  //   float y0 = floor(cursor_y + yo + glyph->metrics.vertBearingY / mult);
  //   float x1 = x0 + glyph->metrics.width;
  //   float y1 = floor(y0 - glyph->metrics.height);

  //   cursor_x += xa;
  //   cursor_y += ya;

  //   hb_position_t x_offset = glyph_pos[i].x_offset / mult;
  //   hb_position_t y_offset = glyph_pos[i].y_offset / mult;
  //   hb_position_t x_advance = glyph_pos[i].x_advance / mult + glyph->metrics.horiBearingX / mult;
  //   hb_position_t y_advance = glyph_pos[i].y_advance / mult + glyph->metrics.horiBearingY / mult;

  //   FT_Vector* pts = glyph->outline.points;
  //   short n_pts = glyph->outline.n_points;

  //   uint32_t offset = geo.offset();

  //   for (int i = 0; i < n_pts; i++) {
  //     geo.push_vertex({ vec2{x0 + (float)pts[i].x / mult, -y0 - (float)pts[i].y / mult} });
  //   }

  //   for (int i = 2; i < n_pts; i++) {
  //     geo.push_indices({ offset + i - 2, offset + i - 1, offset + i });
  //   }

  //   Box box = { vec2{ (float)(cursor_x + x_offset), (float)(cursor_y + y_offset) }, vec2{ (float)(cursor_x + x_offset + glyph->metrics.width / 64), (float)(cursor_y + y_offset + glyph->metrics.height / 64) } };

  //   //cursor_x += x_advance;
  //   //cursor_y += y_advance;

  //   //geo.push_quad(box, vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
  // }

  return geo;
}
