#pragma once

#include "../renderer/texture.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class FontManager {
public:
  FontManager(const FontManager&) = delete;
  FontManager(FontManager&&) = delete;

  static inline FontManager* get() { return s_instance; }
  static inline Texture* get_texture() { return s_instance->m_texture; }

  static void init();
  static void shutdown();

  static void load_font(const unsigned char* buffer, long buffer_size);
private:
  FontManager() = default;
  ~FontManager() = default;

  void load_face(const unsigned char* buffer, long buffer_size);
private:
  FT_Library m_library;
  // TODO: replace with face map
  FT_Face m_face;
  Texture* m_texture;
private:
  static FontManager* s_instance;
};
