#pragma once

#include "../entity.h"

#include "../../../renderer/geometry/path.h"

namespace Graphick::Entities {

  class NewElementEntity : public Entity {
  public:
    using Path = Render::Geometry::Path;
  public:
    NewElementEntity(Path& path);
    NewElementEntity(Path& path, const vec4& color);
    ~NewElementEntity() = default;

    inline virtual TransformComponent* transform() override { return &m_transform; }
    inline virtual const TransformComponent* transform() const override { return &m_transform; }

    virtual void render(const RenderingOptions& options) const override;
  private:
    Path m_path;
    vec4 m_color;

    // Components
    TransformComponent m_transform;
  };

}
