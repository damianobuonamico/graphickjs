#include "text_entity.h"

TextEntity::TextEntity(const vec2& position)
  : m_transform(this, position), m_text("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890=@", "Roboto") {}

void TextEntity::render(const RenderingOptions& options) const {
  Geometry geo;
  geo.push_quad(m_transform.position().get(), 10.0f, vec4{ 0.9f, 0.3f, 0.3f, 1.0f });
  Renderer::draw(geo);

  Renderer::draw(m_text.geometry());
  //Texture* tex = FontManager::get_texture();
  //Renderer::draw(*tex);
}
