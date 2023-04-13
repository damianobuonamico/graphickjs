#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

class FontManager {
public:
  FontManager(const FontManager&) = delete;
  FontManager(FontManager&&) = delete;

  static inline FontManager* get() { return s_instance; }

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
private:
  static FontManager* s_instance;
};
