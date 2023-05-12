#include "new_element_entity.h"

#include "../../../renderer/new_renderer.h"

namespace Graphick::Entities {

  NewElementEntity::NewElementEntity(Path& path) :
    m_transform(this),
    m_path(path),
    m_color{ (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1.0f } {}

  NewElementEntity::NewElementEntity(Path& path, const vec4& color) :
    m_transform(this),
    m_path(path),
    m_color(color) {}

  void NewElementEntity::render(const RenderingOptions& options) const {
    if (m_path.empty()) return;

    Graphick::Render::Renderer::draw(m_path, m_color);
    Graphick::Render::Renderer::draw(m_path.outline_geo());
  }

}
