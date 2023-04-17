#pragma once

#include "font.h"
#include "../../renderer/geometry/geometry.h"

#include <string>

class Text {
public:
  Text(const std::string& text, const std::string& font);
  ~Text();

  Geometry geometry() const;
private:
  bool shape() const;
private:
  mutable bool m_shaped = false;
  // mutable hz_buffer_t m_buffer;

  std::string m_font;
  std::string m_text;

  hb_buffer_t* m_buffer;
};