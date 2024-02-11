#include "scene.h"

#include "entity.h"

#include "../input/input_manager.h"
#include "../input/tools/pen_tool.h"

#include "../../renderer/renderer.h"
#include "../../renderer/geometry/path.h"

#include "../../history/command_history.h"
#include "../../history/commands.h"

// TEMP
#include "../../math/matrix.h"
#include "../../math/math.h"

#include "../../utils/debugger.h"
#include "../../utils/console.h"
#include "../../utils/misc.h"

namespace Graphick::Editor {

  Scene::Scene() : selection(this), history(this) {}

  Scene::Scene(const Scene& other) :
    m_entities(other.m_entities),
    m_order(other.m_order),
    selection(this),
    history(this),
    viewport(other.viewport)
  {
    m_registry.assign(other.m_registry.data(), other.m_registry.data() + other.m_registry.size(), other.m_registry.released());
  }

  Scene::Scene(Scene&& other) noexcept :
    m_registry(std::move(other.m_registry)),
    m_entities(std::move(other.m_entities)),
    selection(this),
    history(this) {}

  Scene::~Scene() {}

  bool Scene::has_entity(const uuid id) const {
    return m_entities.find(id) != m_entities.end();
  }

  Entity Scene::get_entity(const uuid id) {
    return { m_entities.at(id), this };
  }

  Entity Scene::create_entity(const std::string& tag, const bool generate_tag) {
    io::EncodedData data;
    uuid id = uuid();

    data.component_id(IDComponent::component_id)
      .uuid(id);

    if (generate_tag) {
      data.component_id(TagComponent::component_id)
        .string(tag.empty() ? "Entity " + std::to_string(m_entity_tag_number) : tag);
    }

    data.component_id(CategoryComponent::component_id)
      .uint8(static_cast<uint8_t>(CategoryComponent::None));

    history.add(
      id,
      Action::Property::Entity,
      data
    );

    // Entity entity = { m_registry.create(), this };

    // uuid id = uuid();

    // entity.add_component<IDComponent>(id);

    // if (generate_tag) {
    //   entity.add_component<TagComponent>(tag.empty() ? "Entity " + std::to_string(m_entity_tag_number) : tag);
    //   m_entity_tag_number++;
    // }

    // m_entities[id] = entity;
    // m_order.push_back(entity);

    return get_entity(id);
  }

  Entity Scene::create_element() {
    io::EncodedData data;
    uuid id = uuid();

    data.component_id(IDComponent::component_id)
      .uuid(id);

    data.component_id(TagComponent::component_id)
      .string("Element " + std::to_string(m_entity_tag_number));

    data.component_id(CategoryComponent::component_id)
      .uint8(static_cast<uint8_t>(CategoryComponent::Selectable));

    data.component_id(PathComponent::component_id)
      .uint32(0);

    data.component_id(TransformComponent::component_id)
      .mat2x3(mat2x3(1.0f));

    history.add(
      id,
      Action::Property::Entity,
      data
    );

    // Entity entity = create_entity("Element" + std::to_string(m_entity_tag_number));

    // PathComponent& path_component = entity.add_component<PathComponent>();

    // entity.add_component<CategoryComponent>(CategoryComponent::Selectable);
    // entity.add_component<TransformComponent>(entity.id(), &path_component);

    // entity.encode();

    return get_entity(id);
  }

  Entity Scene::create_element(const Renderer::Geometry::Path& path) {
    io::EncodedData data;
    uuid id = uuid();

    data.component_id(IDComponent::component_id)
      .uuid(id);

    data.component_id(TagComponent::component_id)
      .string("Element " + std::to_string(m_entity_tag_number));

    data.component_id(CategoryComponent::component_id)
      .uint8(static_cast<uint8_t>(CategoryComponent::Selectable));

    data.component_id(PathComponent::component_id);
    path.encode(data);

    data.component_id(TransformComponent::component_id)
      .mat2x3(mat2x3(1.0f));

    history.add(
      id,
      Action::Property::Entity,
      data
    );

    // Entity entity = create_entity("Element" + std::to_string(m_entity_tag_number));

    // PathComponent& path_component = entity.add_component<PathComponent>(std::move(path));

    // entity.add_component<CategoryComponent>(CategoryComponent::Selectable);
    // entity.add_component<TransformComponent>(entity.id(), &path_component);

    return get_entity(id);
  }

  void Scene::delete_entity(Entity entity) {
    history.remove(
      entity.id(),
      Action::Property::Entity,
      entity.encode()
    );
  }

  void Scene::delete_entity(uuid id) {
    delete_entity(get_entity(id));
  }

  uuid Scene::entity_at(const vec2 position, bool deep_search, float threshold) {
    const double zoom = viewport.zoom();
    threshold /= zoom;

    GK_DEBUGGER_DRAW(rect{ position - threshold - GK_POINT_EPSILON / zoom, position + threshold + GK_POINT_EPSILON / zoom });

    for (auto it = m_order.rbegin(); it != m_order.rend(); it++) {
      if (m_registry.all_of<PathComponent, TransformComponent>(*it)) {
        const auto& path = m_registry.get<PathComponent>(*it).data;
        const TransformComponent& transform = m_registry.get<TransformComponent>(*it);
        // const vec2 transformed_pos = transform.revert(position);
        // const vec2 transformed_threshold = vec2{ threshold } / Math::decompose(transform.get()).scale;
        const uuid id = m_registry.get<IDComponent>(*it).id;

        bool has_fill = false;
        bool has_stroke = false;

        Renderer::Fill fill;
        Renderer::Stroke stroke;

        if (m_registry.all_of<FillComponent>(*it)) {
          has_fill = true;
          auto& fill_component = m_registry.get<FillComponent>(*it);

          fill = Renderer::Fill{ fill_component.color.get(), fill_component.rule, 0.0f };
        }

        if (m_registry.all_of<StrokeComponent>(*it)) {
          has_stroke = true;
          auto& stroke_component = m_registry.get<StrokeComponent>(*it);

          stroke = Renderer::Stroke{ stroke_component.color.get(), stroke_component.cap, stroke_component.join, stroke_component.width.get(), stroke_component.miter_limit.get(), 0.0f };
        }

        // if (path.is_point_inside_path(transformed_pos, m_registry.all_of<FillComponent>(*it), deep_search && selection.has(id), transformed_threshold)) {
        if (path.is_point_inside_path(position, has_fill ? &fill : nullptr, has_stroke ? &stroke : nullptr, transform.get(), threshold, zoom)) {
          return id;
        }
      }
    }

    return { 0 };
  }

  std::unordered_map<uuid, Selection::SelectionEntry> Scene::entities_in(const Math::rect& rect, bool deep_search) {
    OPTICK_EVENT();

    std::unordered_map<uuid, Selection::SelectionEntry> entities;
    std::unordered_set<uuid> vertices;

#if 0
    auto view = get_all_entities_with<IDComponent, PathComponent, TransformComponent>();

    for (entt::entity entity : view) {
      OPTICK_EVENT("entity_in_rect");

      auto components = view.get<IDComponent, PathComponent, TransformComponent>(entity);

      const uuid id = std::get<0>(components).id;
      const Renderer::Geometry::Path& path = std::get<1>(components).path;
      const TransformComponent& transform_component = std::get<2>(components);
      const mat2x3 transform = transform_component.get();

      // vec2 r1 = transform / rect.min;
      // vec2 r2 = transform / vec2{ rect.max.x, rect.min.y };
      // vec2 r3 = transform / vec2{ rect.min.x, rect.max.y };
      // vec2 r4 = transform / rect.max;

      // std::tuple<vec2, vec2, vec2, vec2> rect_points = { r1, r2, r3, r4 };

      // TODO: add constructor for 4 vec2s
      // Math::rect selection_rect = {
      //   min(min(r1, r2), min(r3, r4)),
      //   max(max(r1, r2), max(r3, r4))
      // };

      float angle = Math::rotation(transform);

      if (Math::is_almost_zero(std::fmodf(angle, MATH_F_TWO_PI))) {
        Math::rect selection_rect = transform / rect;

        if (deep_search) {
          vertices.clear();

          if (Math::is_rect_in_rect(transform_component.bounding_rect(), rect)) {
            entities.insert({ id, Selection::SelectionElementEntry{ { 0 } } });
            continue;
          }

          vertices.reserve(path.segments().size() + 1);

          if (path.intersects(selection_rect, vertices)) {
            entities.insert({ id, Selection::SelectionElementEntry{ vertices } });
          }
        } else {
          if (path.intersects(selection_rect)) {
            entities.insert({ id, Selection::SelectionElementEntry{ { 0 } } });
          }
        }
      } else {
        if (deep_search) {
          vertices.clear();

          if (Math::is_rect_in_rect(transform_component.bounding_rect(), rect)) {
            entities.insert({ id, Selection::SelectionElementEntry{ { 0 } } });
            continue;
          }

          vertices.reserve(path.segments().size() + 1);

          if (path.intersects(rect, transform, vertices)) {
            entities.insert({ id, Selection::SelectionElementEntry{ vertices } });
          }
        } else {
          if (path.intersects(rect, transform)) {
            entities.insert({ id, Selection::SelectionElementEntry{ { 0 } } });
          }
        }
      }

    }

#endif
    return entities;
  }

  void Scene::render() const {
    GK_TOTAL("Scene::render");
    OPTICK_EVENT();

    const Math::rect visible_rect = viewport.visible();

    Renderer::Renderer::begin_frame({
      Math::round(vec2(viewport.size()) * viewport.dpr()),
      viewport.position(),
      viewport.zoom() * viewport.dpr(),
      viewport.dpr(),
      vec4{1.0f, 1.0f, 1.0f, 1.0f}
      });

    const size_t z_far = m_order.size() * 2 + 1;
    float z_index = 1.0f;

    // TODO: maybe check for rehydration only if actions performed are destructive
    bool should_rehydrate = true;

    {
      OPTICK_EVENT("Render Entities");

      auto& selected = selection.selected();
      auto& temp_selected = selection.temp_selected();

      bool draw_vertices = tool_state.active().is_in_category(Input::Tool::CategoryDirect);

      for (auto it = m_order.rbegin(); it != m_order.rend(); it++) {
        if (!m_registry.all_of<IDComponent, PathComponent, TransformComponent>(*it)) continue;

        auto components = m_registry.get<IDComponent, PathComponent, TransformComponent>(*it);

        const uuid id = std::get<0>(components).id;
        const Renderer::Geometry::Path& path = std::get<1>(components).data;
        const TransformComponent& transform = std::get<2>(components);

        if (!has_entity(id)) return;

        if (should_rehydrate) {
          // path.rehydrate_cache();
        }

        bool has_stroke = m_registry.all_of<StrokeComponent>(*it);
        std::optional<StrokeComponent> stroke = has_stroke ? std::optional<StrokeComponent>(m_registry.get<StrokeComponent>(*it)) : std::nullopt;

        rect entity_rect = transform.approx_bounding_rect();

        if (has_stroke) {
          entity_rect.min -= vec2{ stroke->width.get() };
          entity_rect.max += vec2{ stroke->width.get() };
        }

        if (!Math::does_rect_intersect_rect(entity_rect, visible_rect)) continue;

        mat2x3 transform_matrix = transform.get();

        bool has_fill = m_registry.all_of<FillComponent>(*it);
        std::optional<FillComponent> fill = has_fill ? std::optional<FillComponent>(m_registry.get<FillComponent>(*it)) : std::nullopt;

        if (has_fill && !fill->visible) has_fill = false;
        if (has_stroke && !stroke->visible) has_stroke = false;

        if (has_fill && has_stroke) {
          Renderer::Renderer::draw(
            path,
            Renderer::Stroke{ stroke->color.get(), stroke->cap, stroke->join, stroke->width.get(), stroke->miter_limit.get(), z_index / z_far },
            Renderer::Fill{ fill->color.get(), fill->rule, (z_index + 1) / z_far },
            transform_matrix
          );

          z_index += 2;
        } else if (has_fill) {
          Renderer::Renderer::draw(
            path,
            Renderer::Fill{ fill->color.get(), fill->rule, z_index / z_far },
            transform_matrix
          );

          z_index += 1;
        } else if (has_stroke) {
          Renderer::Renderer::draw(
            path,
            Renderer::Stroke{ stroke->color.get(), stroke->cap, stroke->join, stroke->width.get(), stroke->miter_limit.get(), z_index / z_far },
            transform_matrix
          );

          z_index += 1;
        }

        if (selected.find(id) != selected.end() || temp_selected.find(id) != temp_selected.end()) {
          Renderer::Renderer::draw_outline(id, path, transform_matrix, draw_vertices);
        }

        // Math::rect bounding_rect = path.bounding_rect();
        // std::vector<Math::rect> lines = Math::lines_from_rect(bounding_rect);
        // Renderer::Geometry::Path rect;
        // rect.move_to(lines[0].min);

        // for (auto& line : lines) {
        //   rect.line_to(line.max);
        // }

        // Renderer::Renderer::draw_outline(id, rect, position);
      }
    }

    // {
    //   std::vector<Math::rect> lines = Math::lines_from_rect(viewport.visible());
    //   Renderer::Geometry::Path rect;
    //   rect.move_to(lines[0].min);

    //   for (auto& line : lines) {
    //     rect.line_to(line.max);
    //   }

    //   Renderer::Renderer::draw_outline(0, rect, { 0.0f, 0.0f });
    // }

    {
      OPTICK_EVENT("Render Overlays");

      tool_state.render_overlays(viewport.zoom());
    }

    Renderer::Renderer::end_frame();

    GK_DEBUGGER_RENDER(vec2(viewport.size()) * viewport.dpr());
  }

  void Scene::add(const uuid id, const io::EncodedData& encoded_data) {
    Entity entity = { m_registry.create(), this, encoded_data };

    m_entities[id] = entity;
    m_order.push_back(entity);
  }

  void Scene::remove(const uuid id) {
    auto it = m_entities.find(id);
    if (it == m_entities.end()) return;

    entt::entity entity = it->second;

    selection.deselect(id);
    m_registry.destroy(entity);
    m_entities.erase(it);
    m_order.erase(std::remove(m_order.begin(), m_order.end(), static_cast<entt::entity>(entity)), m_order.end());
  }

}
