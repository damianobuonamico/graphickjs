#include "text_entity.h"

#include "../../font_manager.h"

void TextEntity::render(const RenderingOptions& options) const {
  Geometry geo;
  geo.push_quad(m_transform.position().get(), 10.0f, vec4{ 0.9f, 0.3f, 0.3f, 1.0f });
  Renderer::draw(geo);

  Texture* tex = FontManager::get_texture();
  Renderer::draw(*tex);
}
