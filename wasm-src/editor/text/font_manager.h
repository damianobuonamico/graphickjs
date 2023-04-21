#pragma once

#include "font.h"

#include <unordered_map>
#include <string>
#include <memory>

class FontManager {
public:
  FontManager(const FontManager&) = delete;
  FontManager(FontManager&&) = delete;

  static inline FontManager* get() { return s_instance; }

  static void init();
  static void shutdown();

  static void load_font(const unsigned char* buffer, long buffer_size);
  static std::weak_ptr<Font> get_font(const std::string& name);
private:
  FontManager() = default;
  ~FontManager() = default;
private:
  std::unordered_map<std::string, std::shared_ptr<Font>> m_fonts;
private:
  static FontManager* s_instance;
};
