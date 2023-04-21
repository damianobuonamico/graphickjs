#include "font_manager.h"

#include "../../utils/console.h"
#include "../../io/woff2/include/woff2/output.h"
#include "../../io/woff2/include/woff2/decode.h"

#include <assert.h>
#include <vector>

FontManager* FontManager::s_instance = nullptr;

void FontManager::init() {
  assert(!s_instance);
  s_instance = new FontManager();
}

void FontManager::shutdown() {
  delete s_instance;
}

void FontManager::load_font(const unsigned char* buffer, long buffer_size) {
  std::shared_ptr<Font> font = std::make_shared<Font>(buffer, (size_t)buffer_size);
  get()->m_fonts.insert({ "Roboto", font });
}

std::weak_ptr<Font> FontManager::get_font(const std::string& name) {
  auto it = get()->m_fonts.find(name);

  if (it == get()->m_fonts.end()) {
    return {};
  }

  return { it->second };
}
