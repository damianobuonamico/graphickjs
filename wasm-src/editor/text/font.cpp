#include "font.h"

#include "../../io/woff2/include/woff2/output.h"
#include "../../io/woff2/include/woff2/decode.h"
#include "../../renderer/geometry/earcut.h"
#include "../../utils/console.h"

#include <array>

#define WOFF2_SIGNATURE 0x774F4632

Font::Font(const uint8_t* buffer, size_t buffer_size) {
  if (buffer_size < 4) {
    console::error("Invalid font buffer size!");
  }

  if ((buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3]) == WOFF2_SIGNATURE) {
    m_buffer_size = woff2::ComputeWOFF2FinalSize(buffer, buffer_size);
    if (m_buffer_size == 0) {
      console::error("Failed to compute WOFF2 final size!");
    }

    m_buffer = new uint8_t[m_buffer_size];
    woff2::WOFF2MemoryOut out(m_buffer, m_buffer_size);

    bool success = woff2::ConvertWOFF2ToTTF(buffer, buffer_size, &out);
    if (!success) {
      console::error("Failed to convert WOFF2 to TTF!");
    }

    m_buffer_size = out.Size();
  } else {
    m_buffer_size = buffer_size;
    m_buffer = new uint8_t[m_buffer_size];
    memcpy(m_buffer, buffer, m_buffer_size);
  }

  if (!stbtt_InitFont(&m_font_info, m_buffer, 0)) {
    console::error("Failed to initialize font info!");
  }

  m_blob = hb_blob_create((const char*)m_buffer, m_buffer_size, HB_MEMORY_MODE_WRITABLE, m_buffer, nullptr);
  m_face = hb_face_create(m_blob, 0);
  m_font = hb_font_create(m_face);
}

Font::~Font() {
  hb_font_destroy(m_font);
  hb_face_destroy(m_face);
  hb_blob_destroy(m_blob);

  delete[] m_buffer;
}

static bool is_winding_clockwise(std::vector<std::array<float, 2>>& contour, int min_index) {
  std::array<float, 2> a = contour[min_index > 0 ? min_index - 1 : contour.size() - 1];
  std::array<float, 2> b = contour[min_index];
  std::array<float, 2> c = contour[min_index < contour.size() - 1 ? min_index + 1 : 0];

  float det = (b[0] * c[1] + a[0] * b[1] + a[1] * c[0]) - (a[1] * b[0] + b[1] * c[0] + a[0] * c[1]);
  return det >= 0;
}

static vec4 colors[6] = {
  { 1.0f, 0.0f, 0.0f, 0.3f },
  { 0.0f, 1.0f, 0.0f, 0.3f },
  { 0.0f, 0.0f, 1.0f, 0.3f },
  { 1.0f, 1.0f, 0.0f, 0.3f },
  { 1.0f, 0.0f, 1.0f, 0.3f },
  { 0.0f, 1.0f, 1.0f, 0.3f }
};

const Geometry& Font::request_glyph(hb_codepoint_t glyph_id, float scale) {
  // Check if glyph is already cached
  if (m_glyphs.find(glyph_id) != m_glyphs.end()) {
    return m_glyphs.at(glyph_id);
  }

  // Get vertex array
  stbtt_vertex* vertices;
  int vertices_len = stbtt_GetGlyphShape(&m_font_info, glyph_id, &vertices);

  using Point = std::array<float, 2>;
  std::vector<std::vector<Point>> contours;
  std::vector<bool> windings;

  int contour_index = 0;
  int min_index = 0;
  short min_x = std::numeric_limits<short>::max();
  short min_y = std::numeric_limits<short>::max();

  for (int i = 0; i < vertices_len; i++) {
    stbtt_vertex& vertex = vertices[i];

    // Create new contour on move command, reserve more memory than needed to avoid reallocation
    if (vertex.type == STBTT_vmove) {
      contours.push_back({});
      contour_index = contours.size() - 1;
      contours[contour_index].reserve(vertices_len);

      // Calculate previous contour winding order
      if (contour_index > 0) {
        windings.push_back(is_winding_clockwise(contours[contour_index - 1], min_index));

        min_index = 0;
        min_x = min_y = std::numeric_limits<short>::max();
      }
    } else if (vertex.type == STBTT_vcurve) {
      contours[contour_index].push_back({ (float)vertex.cx * scale, -(float)vertex.cy * scale });
    } else if (vertex.type == STBTT_vcubic) {
      contours[contour_index].push_back({ (float)vertex.cx * scale, -(float)vertex.cy * scale });
      contours[contour_index].push_back({ (float)vertex.cx1 * scale, -(float)vertex.cy1 * scale });
    }

    contours[contour_index].push_back({ (float)vertex.x * scale, -(float)vertex.y * scale });

    // Find point with smallest y (and x in case of ties) to determine winding order
    if (vertex.y > min_y || (vertex.y == min_y && vertex.x >= min_x)) continue;

    min_index = (int)contours[contour_index].size() - 1;
    min_x = vertex.x;
    min_y = vertex.y;
  }

  if (!contours.empty()) {
    windings.push_back(is_winding_clockwise(contours[contour_index], min_index));
  }

  // Calculate winding order

  console::log("contours", contours.size());
  for (int i = 0; i < windings.size(); i++) {
    console::log(std::to_string(i), windings[i] ? "CW" : "CCW");
  }

  Geometry geo;

  for (int i = 0; i < contours.size(); i++) {
    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(std::vector<std::vector<Point>>{ contours[i] });

    uint32_t offset = geo.offset();
    geo.reserve(contours[i].size(), indices.size());

    for (int j = 0; j < contours[i].size(); j++) {
      // geo.push_vertex({ vec2{ contours[i][j][0], contours[i][j][1] }, colors[i % 6] });
      geo.push_vertex({ vec2{ contours[i][j][0], contours[i][j][1] } });
    }

    for (int j = 0; j < indices.size(); j++) {
      geo.push_index(offset + indices[j]);
    }

    for (int j = 0; j < contours[i].size(); j++) {
      // geo.push_quad(vec2{ contours[i][j][0], contours[i][j][1] }, 0.5f, vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
      geo.push_quad(vec2{ contours[i][j][0], contours[i][j][1] }, 0.5f, vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    }
  }
#if 0
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
#endif

  stbtt_FreeShape(&m_font_info, vertices);

  m_glyphs.insert({ glyph_id, geo });
  return m_glyphs.at(glyph_id);
}
