/**
 * @file renderer/properties.cpp
 * @brief Contains the implementations of the properties used to render a path.
 */

#include "properties.h"

#include "../io/encode/encode.h"

namespace graphick::renderer {

Paint::Paint(const uuid paint_id, const Type type) : m_type(type), m_id(paint_id)
{
  if (m_type == Type::ColorPaint) {
    m_type = Type::SwatchPaint;
  }
}

Paint::Paint(io::DataDecoder& decoder)
{
  const auto [is_color, is_gradient, is_texture] = decoder.bitfield<3>();

  if (is_color) {
    m_type = Type::ColorPaint;
    m_color = decoder.color();
  } else if (is_gradient) {
    m_type = Type::GradientPaint;
    m_id = decoder.uuid();
  } else if (is_texture) {
    m_type = Type::TexturePaint;
    m_id = decoder.uuid();
  } else {
    m_type = Type::SwatchPaint;
    m_id = decoder.uuid();
  }
}

io::EncodedData& Paint::encode(io::EncodedData& data) const
{
  data.bitfield({is_color(), is_gradient(), is_texture()});

  if (is_color()) {
    data.color(m_color);
  } else {
    data.uuid(m_id);
  }

  return data;
}

}  // namespace graphick::renderer
