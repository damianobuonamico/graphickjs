#include "text_entity.h"

void TextEntity::render(const RenderingOptions& options) const {
  Geometry geo;
  geo.push_quad(m_transform.position().get(), 10.0f, vec4{ 0.9f, 0.3f, 0.3f, 1.0f });
  Renderer::draw(geo);
}
