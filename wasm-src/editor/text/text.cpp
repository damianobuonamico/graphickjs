#include "text.h"

#include "font_manager.h"
#include "../../utils/console.h"
#include "../../math/vector.h"
#include "../../math/math.h"
#include "../../renderer/geometry/earcut.h"
#include <array>

Text::Text(const std::string& text, const std::string& font): m_text(text), m_font(font) {
  m_buffer = hb_buffer_create();
  hb_buffer_add_utf8(m_buffer, text.c_str(), (int)text.size(), 0, (int)text.size());

  hb_buffer_set_direction(m_buffer, HB_DIRECTION_LTR);
  hb_buffer_set_script(m_buffer, HB_SCRIPT_LATIN);
  hb_buffer_set_language(m_buffer, hb_language_from_string("en", -1));

  shape();
}

Text::~Text() {
  hb_buffer_destroy(m_buffer);
}

bool Text::shape() const {
  std::weak_ptr<Font> font = FontManager::get_font(m_font);
  if (font.expired()) return false;

  hb_shape(font.lock()->get().font, m_buffer, nullptr, 0);
  m_shaped = true;

  return true;
}

Geometry Text::geometry() const {
  Geometry geo;

  if (!m_shaped) {
    if (!shape()) return geo;
  }

  std::weak_ptr<Font> font_ptr = FontManager::get_font(m_font);
  if (font_ptr.expired()) return false;

  std::shared_ptr<Font> font = font_ptr.lock();

  stbtt_fontinfo* font_info = font->get().font_info;
  float scale = stbtt_ScaleForPixelHeight(font_info, 64);

  unsigned int glyph_count;
  hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(m_buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(m_buffer, &glyph_count);

  float x = 0.0f;
  float y = 0.0f;

  for (unsigned int i = 0; i < glyph_count; i++) {
    hb_codepoint_t glyph_id = glyph_info[i].codepoint;

    const Geometry& glyph_geo = font->request_glyph(glyph_id, scale);

    auto& verts = glyph_geo.vertices();
    geo.reserve(verts.size(), glyph_geo.indices().size());
    uint32_t geo_off = geo.offset();

    float x_offset = (float)glyph_pos[i].x_offset * scale;
    float y_offset = (float)glyph_pos[i].y_offset * scale;
    float x_advance = (float)glyph_pos[i].x_advance * scale;
    float y_advance = (float)glyph_pos[i].y_advance * scale;

    for (const Vertex& vert : verts) {
      geo.push_vertex({ vec2{ vert.position.x + x + x_offset, vert.position.y + y + y_offset }, vert.color });
    }

    for (uint32_t index : glyph_geo.indices()) {
      geo.push_index(geo_off + index);
    }

    x += x_advance;
    y += y_advance;

#if 0
    stbtt_vertex* vertices;
    int vertices_len = stbtt_GetGlyphShape(font_info, glyph_id, &vertices);

    using Point = std::array<float, 2>;
    std::vector<std::vector<Point>> polygon;

    size_t contour_index = 0;
    uint32_t offset = geo.offset();

    float x_offset = (float)glyph_pos[i].x_offset * scale;
    float y_offset = (float)glyph_pos[i].y_offset * scale;
    float x_advance = (float)glyph_pos[i].x_advance * scale;
    float y_advance = (float)glyph_pos[i].y_advance * scale;


    for (int j = 0; j < vertices_len; j++) {
      auto& vertex = vertices[j];
      if (vertex.type == STBTT_vmove) {
        polygon.push_back({});
        contour_index = polygon.size() - 1;
      }

      polygon[contour_index].push_back({ { x + x_offset + (float)vertex.x * scale, -(float)vertex.y * scale } });
      geo.push_vertex({ vec2{ x + x_offset + (float)vertex.x * scale, -(float)vertex.y * scale } });
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

    for (int h = 1; h < polygon.size(); h++) {
      bool same_winding = windings[offs] == windings[h];
      bool is_in_box = is_box_in_box(boxes[h], boxes[offs]);
      bool is_in_box_rev = is_box_in_box(boxes[offs], boxes[h]);
      if (!same_winding && is_in_box_rev) {

      } else if (same_winding || !is_in_box) {
        polygons.push_back({});
        polygons.back().insert(polygons.back().end(), polygon.begin() + offs, polygon.begin() + h);
        offs = h;
      }
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
      for (int k = 0; k < polygons.size(); k++) {
        auto& poly = polygons[k];

        std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(poly);

        //if (k == 2) {
        for (auto& index : indices) {
          geo.push_index(oo + index);
        }
        //}

        for (auto& contour : poly) {
          oo += contour.size();
        }
      }
    }

    x += x_advance;

    stbtt_FreeShape(font_info, vertices);
#endif
  }

  return geo;
}
