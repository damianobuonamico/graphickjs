#include "font_manager.h"

#include "../../utils/console.h"
#include "../../utils/woff2/include/woff2/output.h"
#include "../../utils/woff2/include/woff2/decode.h"

// #include <hb.h>
// #define HZ_IMPLEMENTATION
// #include "../../utils/hamza/hz.h"

#include <assert.h>
#include <vector>

FontManager* FontManager::s_instance = nullptr;

void FontManager::init() {
  assert(!s_instance);
  s_instance = new FontManager();

  // hz_error_t error = hz_setup(HZ_QUERY_CPU_FOR_SIMD);
  // if (error != HZ_OK) {
    // console::error("Failed to setup Hamza library", error);
  // }

  // FT_Error error = FT_Init_FreeType(&get()->m_library);
  // if (error) {
  //   console::error("Failed to initialize FreeType library", error);
  // }
}

void FontManager::shutdown() {
  // FT_Done_FreeType(get()->m_library);
  // hz_cleanup();
  delete s_instance;
}

void FontManager::load_font(const unsigned char* buffer, long buffer_size) {
  std::shared_ptr<Font> font = std::make_shared<Font>(buffer, (size_t)buffer_size);
  get()->m_fonts.insert({ "Roboto", font });
  // get()->load_face(buffer, buffer_size);
}

std::weak_ptr<Font> FontManager::get_font(const std::string& name) {
  auto it = get()->m_fonts.find(name);

  if (it == get()->m_fonts.end()) {
    return {};
  }

  return { it->second };
}

void FontManager::load_face(const unsigned char* buffer, long buffer_size) {
  // uint8_t* buf = new uint8_t[buffer_size * 5];
  // woff2::WOFF2MemoryOut out(buf, buffer_size * 5);
  // bool success = ConvertWOFF2ToTTF(buffer, buffer_size, &out);

  // hb_buffer_t* hb_buf = hb_buffer_create();
  // hb_buffer_add_utf8(hb_buf, "abcdefghijklmnopqrstuvwxyz", -1, 0, -1);

  // hb_buffer_set_direction(hb_buf, HB_DIRECTION_LTR);
  // hb_buffer_set_script(hb_buf, HB_SCRIPT_LATIN);
  // hb_buffer_set_language(hb_buf, hb_language_from_string("en", -1));

  // hb_blob_t* blob = hb_blob_create((const char*)buf, out.Size(), HB_MEMORY_MODE_WRITABLE, buf, free);
  // hb_face_t* face = hb_face_create(blob, 0);
  // hb_font_t* font = hb_font_create(face);

  // hb_shape(font, hb_buf, nullptr, 0);

  // unsigned int glyph_count;
  // hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(hb_buf, &glyph_count);
  // hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(hb_buf, &glyph_count);

  // console::log("glyph count", glyph_count);

  // hb_position_t cursor_x = 0;
  // hb_position_t cursor_y = 0;
  // for (unsigned int i = 0; i < glyph_count; i++) {
  //   hb_codepoint_t glyphid = glyph_info[i].codepoint;
  //   hb_position_t x_offset = glyph_pos[i].x_offset;
  //   hb_position_t y_offset = glyph_pos[i].y_offset;
  //   hb_position_t x_advance = glyph_pos[i].x_advance;
  //   hb_position_t y_advance = glyph_pos[i].y_advance;

  //   /* draw_glyph(glyphid, cursor_x + x_offset, cursor_y + y_offset); */
  //   cursor_x += x_advance;
  //   cursor_y += y_advance;
  // }

  // FT_Error error = FT_New_Memory_Face(m_library, buf, (FT_Long)out.Size(), 0, &m_face);
  // if (error) {
  //   console::error("Failed to load font face", error);
  //   return;
  // }

  // error = FT_Set_Pixel_Sizes(m_face, 0, 64);
  // if (error) {
  //   console::error("Failed to set font size", error);
  //   return;
  // }

  // FT_UInt glyph_index = FT_Get_Char_Index(m_face, '@');
  // error = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER);
  // if (error) {
  //   console::error("Failed to load glyph", error);
  //   return;
  // }

  // error = FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_SDF);
  // if (error) {
  //   console::error("Failed to render glyph", error);
  //   return;
  // }

  // FT_GlyphSlot slot = m_face->glyph;
  // m_texture = new Texture(slot->bitmap.buffer, slot->bitmap.width, slot->bitmap.rows);
}
