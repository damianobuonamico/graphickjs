#include "selection_rect_entity.h" 

void SelectionRectEntity::set(const vec2& position) {
  m_active = true;
  m_transform.position().move_to(position);
  m_transform.size().move_to(vec2{ 0.0f });
}

void SelectionRectEntity::size(const vec2& size) {
  m_transform.size().move_to(size);
}

void SelectionRectEntity::reset() {
  m_active = false;
  m_transform.size().move_to(vec2{ 0.0f });
}

void SelectionRectEntity::tessellate_outline(const vec4& color, RenderingOptions options, Geometry& geo) const {
  if (!m_active || is_zero(m_transform.size().get())) {
    return;
  }

  geo.push_quad_outline(m_transform.bounding_box(), color, m_dashed ? 5.0f / options.zoom : 0.0f);
}

// TODO: check where headers are included to reduce compilation time
void SelectionRectEntity::render(RenderingOptions options) const {
  if (!m_active || is_zero(m_transform.size().get())) {
    return;
  }

  Geometry geo{};
  geo.push_quad(m_transform.bounding_box(), vec4(0.22f, 0.76f, 0.95f, 0.3f));
  Renderer::draw(geo);
}

