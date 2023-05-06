#include "new_element_entity.h"

#include "../../../renderer/renderer.h"

namespace Graphick::Entities {

  NewElementEntity::NewElementEntity(Path& path) :
    m_transform(this),
    m_path(path) {}

  void NewElementEntity::render(const RenderingOptions& options) const {
    if (m_path.empty()) return;

    Renderer::draw(m_path.outline_geo());
  }

}
