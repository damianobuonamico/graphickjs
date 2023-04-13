#include "font_manager.h"

#include "../utils/console.h"
#include "../utils/woff2/include/woff2/output.h"
#include "../utils/woff2/include/woff2/decode.h"

#include <assert.h>
#include <vector>

FontManager* FontManager::s_instance = nullptr;

void FontManager::init() {
  assert(!s_instance);
  s_instance = new FontManager();

  FT_Error error = FT_Init_FreeType(&get()->m_library);
  if (error) {
    console::error("Failed to initialize FreeType library", error);
  }
}

void FontManager::shutdown() {
  FT_Done_FreeType(get()->m_library);
  delete s_instance;
}

void FontManager::load_font(const unsigned char* buffer, long buffer_size) {
  get()->load_face(buffer, buffer_size);
}

void FontManager::load_face(const unsigned char* buffer, long buffer_size) {
  // std::string buf;
  // woff2::WOFF2StringOut out(&buf);
  uint8_t* buf = new uint8_t[buffer_size * 5];
  woff2::WOFF2MemoryOut out(buf, buffer_size * 5);
  ConvertWOFF2ToTTF(buffer, buffer_size, &out);

  FT_Error error = FT_New_Memory_Face(m_library, buf, out.Size(), 0, &m_face);
  if (error) {
    console::error("Failed to load font face", error);
    return;
  }

  error = FT_Set_Pixel_Sizes(m_face, 0, 16);
  if (error) {
    console::error("Failed to set font size", error);
    return;
  }

  FT_UInt glyph_index = FT_Get_Char_Index(m_face, 'A');
  error = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_DEFAULT);
  if (error) {
    console::error("Failed to load glyph", error);
    return;
  }

  error = FT_Render_Glyph(m_face->glyph, ft_render_mode_normal);
  if (error) {
    console::error("Failed to render glyph", error);
    return;
  }
}
