/**
 * @file scene.cpp
 * @brief This file contains the implementation of the Scene class.
 *
 * @todo refactor entity/element creation
 * @todo refactor checks, id and transform should always be present
 * @todo operator StrokeComponent::Renderer::Stroke() to use in entity_at()
 * @todo implement better check for rotated rects to use in entities_in()
 * @todo reimplment cache and maybe check for rehydration only if actions performed are destructive
 * @todo get more than one component at a time in entity
 * @todo refactor and abstract away entity related methods in render()
 * @todo when scene serialization is a thing, implement copy constructor
 */

#include "scene.h"

#include "entity.h"

#include "../input/input_manager.h"
#include "../input/tools/pen_tool.h"

#include "../../renderer/renderer_new.h"

#include "../../math/matrix.h"
#include "../../math/math.h"

#include "../../geom/intersections.h"
#include "../../geom/path.h"

#include "../../utils/debugger.h"
#include "../../utils/console.h"
#include "../../utils/misc.h"

namespace graphick::editor {

  Scene::Scene() : selection(this), history(this) {}

  Scene::Scene(const Scene& other) :
    m_entities(other.m_entities),
    m_order(other.m_order),
    selection(this),
    history(this),
    viewport(other.viewport)
  {}

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

  const Entity Scene::get_entity(const uuid id) const {
    return { m_entities.at(id), const_cast<Scene*>(this) };
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
      .uint8(static_cast<uint8_t>(CategoryComponent::Category::None));

    data.component_id(TransformComponent::component_id)
      .mat2x3(mat2x3(1.0f));

    history.add(
      id,
      Action::Target::Entity,
      std::move(data)
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
      .uint8(static_cast<uint8_t>(CategoryComponent::Category::Selectable));

    data.component_id(PathComponent::component_id)
      .uint32(0);

    data.component_id(TransformComponent::component_id)
      .mat2x3(mat2x3(1.0f));

    history.add(
      id,
      Action::Target::Entity,
      std::move(data)
    );

    // Entity entity = create_entity("Element" + std::to_string(m_entity_tag_number));

    // PathComponent& path_component = entity.add_component<PathComponent>();

    // entity.add_component<CategoryComponent>(CategoryComponent::Selectable);
    // entity.add_component<TransformComponent>(entity.id(), &path_component);

    // entity.encode();

    return get_entity(id);
  }

  Entity Scene::create_element(const geom::Path& path) {
    io::EncodedData data;
    uuid id = uuid();

    data.component_id(IDComponent::component_id)
      .uuid(id);

    data.component_id(TagComponent::component_id)
      .string("Element " + std::to_string(m_entity_tag_number));

    data.component_id(CategoryComponent::component_id)
      .uint8(static_cast<uint8_t>(CategoryComponent::Category::Selectable));

    data.component_id(PathComponent::component_id);
    path.encode(data);

    data.component_id(TransformComponent::component_id)
      .mat2x3(mat2x3(1.0f));

    history.add(
      id,
      Action::Target::Entity,
      std::move(data)
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
      Action::Target::Entity,
      entity.encode()
    );
  }

  void Scene::delete_entity(uuid id) {
    delete_entity(get_entity(id));
  }

  uuid Scene::entity_at(const vec2 position, bool deep_search, float threshold) {
    const double zoom = viewport.zoom();
    threshold /= zoom;

    for (auto it = m_order.rbegin(); it != m_order.rend(); it++) {
      const Entity entity = { *it, this };

      if (entity.has_components<IDComponent, PathComponent, TransformComponent>()) {
        const auto& path = entity.get_component<PathComponent>().data();
        const TransformComponent transform = entity.get_component<TransformComponent>();
        const uuid id = entity.id();

        bool has_fill = false;
        bool has_stroke = false;

        renderer::Fill fill;
        renderer::Stroke stroke;

        if (entity.has_component<FillComponent>()) {
          has_fill = true;
          auto& fill_component = entity.get_component<FillComponent>().fill_TEMP();

          fill = renderer::Fill{ fill_component.color, fill_component.rule, 0.0f };
        }

        if (entity.has_component<StrokeComponent>()) {
          has_stroke = true;
          auto& stroke_component = entity.get_component<StrokeComponent>().stroke_TEMP();

          stroke = renderer::Stroke{ stroke_component.color, stroke_component.cap, stroke_component.join, stroke_component.width, stroke_component.miter_limit, 0.0f };
        }

        if (path.is_point_inside_path(position, has_fill ? &fill : nullptr, has_stroke ? &stroke : nullptr, transform, threshold, zoom, deep_search && selection.has(id))) {
          return id;
        }
      }
    }

    return { 0 };
  }

  Entity Scene::duplicate_entity(const uuid id) {
    Entity entity = get_entity(id);

    auto [new_id, data] = entity.duplicate();

    history.add(
      new_id,
      Action::Target::Entity,
      std::move(data)
    );

    return get_entity(new_id);
  }

  std::unordered_map<uuid, Selection::SelectionEntry> Scene::entities_in(const math::rect& rect, bool deep_search) {
    OPTICK_EVENT();

    std::unordered_map<uuid, Selection::SelectionEntry> entities;
    std::unordered_set<size_t> vertices;

    auto view = get_all_entities_with<IDComponent::Data, TransformComponent::Data>();

    for (entt::entity handle : view) {
      OPTICK_EVENT("entity_in_rect");

      const Entity entity = { handle, this };
      const uuid id = entity.id();
      const TransformComponent transform = entity.get_component<TransformComponent>();

      const float angle = math::rotation(transform.matrix());

      if (math::is_almost_zero(std::fmodf(angle, math::two_pi<float>))) {
        const mat2x3 inverse_transform = transform.inverse();
        const math::rect selection_rect = inverse_transform * rect;

        if (entity.is_element()) {
          if (geom::is_rect_in_rect(transform.bounding_rect(), rect)) {
            entities.insert({ id, Selection::SelectionEntry() });
            continue;
          }

          const PathComponent path = entity.get_component<PathComponent>();

          if (deep_search) {
            vertices.clear();

            if (path.data().intersects(selection_rect, &vertices)) {
              entities.insert({ id, Selection::SelectionEntry{ vertices } });
            }
          } else {
            if (path.data().intersects(selection_rect)) {
              entities.insert({ id, Selection::SelectionEntry() });
            }
          }
        } else {
          if (geom::does_rect_intersect_rect(transform.bounding_rect(), rect)) {
            entities.insert({ id, Selection::SelectionEntry() });
          }
        }
      } else {
        if (entity.is_element()) {
          if (geom::is_rect_in_rect(transform.bounding_rect(), rect)) {
            entities.insert({ id, Selection::SelectionEntry() });
            continue;
          }

          const PathComponent path = entity.get_component<PathComponent>();

          if (deep_search) {
            vertices.clear();

            if (path.data().intersects(rect, transform, &vertices)) {
              entities.insert({ id, Selection::SelectionEntry{ vertices } });
            }
          } else {
            if (path.data().intersects(rect, transform)) {
              entities.insert({ id, Selection::SelectionEntry() });
            }
          }
        } else {
          if (geom::does_rect_intersect_rect(transform.bounding_rect(), rect)) {
            entities.insert({ id, Selection::SelectionEntry() });
          }
        }
      }

    }

    return entities;
  }

  void Scene::render() const {
    GK_TOTAL("Scene::render");
    OPTICK_EVENT();

    const math::rect visible_rect = viewport.visible();

    renderer::Renderer::begin_frame({
      math::round(vec2(viewport.size()) * viewport.dpr()),
      viewport.position(),
      viewport.zoom() * viewport.dpr(),
      viewport.dpr(),
      vec4{0.2f, 0.2f, 0.21f, 1.0f}
    });

    const size_t z_far = m_order.size() * 2 + 1;
    float z_index = 1.0f;
    float tolerance = GK_PATH_TOLERANCE / 2.0f;
    float outline_tolerance = GK_PATH_TOLERANCE / (viewport.zoom() * viewport.dpr());

    bool should_rehydrate = true;

    {
      OPTICK_EVENT("Render Entities");

      auto& selected = selection.selected();
      auto& temp_selected = selection.temp_selected();

      bool draw_vertices = tool_state.active().is_in_category(input::Tool::CategoryDirect);

      for (auto it = m_order.begin(); it != m_order.end(); it++) {
        const Entity entity = { *it, const_cast<Scene*>(this) };
        if (!entity.has_components<IDComponent, PathComponent, TransformComponent>()) continue;

        const uuid id = entity.id();
        const geom::Path& path = entity.get_component<PathComponent>().data();
        const TransformComponent transform = entity.get_component<TransformComponent>();

        if (!has_entity(id)) return;

        if (should_rehydrate) {
          // path.rehydrate_cache();
        }

        bool has_stroke = m_registry.all_of<StrokeComponent::Data>(*it);
        std::optional<StrokeComponent::Data> stroke = has_stroke ? std::optional<StrokeComponent::Data>(m_registry.get<StrokeComponent::Data>(*it)) : std::nullopt;

        rect entity_rect = transform.approx_bounding_rect();

        if (has_stroke) {
          entity_rect.min -= vec2{ stroke->width };
          entity_rect.max += vec2{ stroke->width };
        }

        if (!geom::does_rect_intersect_rect(entity_rect, visible_rect)) continue;

        bool has_fill = m_registry.all_of<FillComponent::Data>(*it);
        std::optional<FillComponent::Data> fill = has_fill ? std::optional<FillComponent::Data>(m_registry.get<FillComponent::Data>(*it)) : std::nullopt;

        if (has_fill && !fill->visible) has_fill = false;
        if (has_stroke && !stroke->visible) has_stroke = false;

        bool is_selected = selected.find(id) != selected.end();
        bool is_temp_selected = temp_selected.find(id) != temp_selected.end();
        bool is_full = false;

        if ((!is_selected && !is_selected) && !has_fill && !has_stroke) continue;

        mat2x3 transform_matrix = transform.matrix();

        auto transformation = math::decompose(transform_matrix);
        float scale = std::max(transformation.scale.x, transformation.scale.y);

        geom::QuadraticPath quadratics = path.to_quadratics(tolerance / scale);

        if (has_fill && has_stroke) {
          renderer::Renderer::draw(
            quadratics,
            renderer::Stroke{ stroke->color, stroke->cap, stroke->join, stroke->width, stroke->miter_limit, z_index / z_far },
            renderer::Fill{ fill->color, fill->rule, (z_index + 1) / z_far },
            transform_matrix
          );

          z_index += 2;
        } else if (has_fill) {
          renderer::Renderer::draw(
            quadratics,
            renderer::Fill{ fill->color, fill->rule, z_index / z_far },
            transform_matrix
          );

          z_index += 1;
        } else if (has_stroke) {
          renderer::Renderer::draw(
            quadratics,
            renderer::Stroke{ stroke->color, stroke->cap, stroke->join, stroke->width, stroke->miter_limit, z_index / z_far },
            transform_matrix
          );

          z_index += 1;
        }

        if (!is_selected && !is_temp_selected) continue;

        std::unordered_set<size_t> selected_vertices;

        if (is_selected) {
          Selection::SelectionEntry entry = selected.at(id);

          is_full = entry.full();

          if (!is_full) {
            selected_vertices = entry.indices;
          }
        }

        if (is_temp_selected && !is_full) {
          Selection::SelectionEntry entry = temp_selected.at(id);

          is_full = entry.full();

          if (!is_full) {
            selected_vertices.insert(temp_selected.at(id).indices.begin(), temp_selected.at(id).indices.end());
          }
        }

        // TEMP
        renderer::Renderer::draw_outline(path, transform, outline_tolerance);
        renderer::Renderer::draw_outline(quadratics, transform, outline_tolerance);
        renderer::Renderer::draw_outline_vertices(
          path, transform,
          is_full ? nullptr : &selected_vertices
        );


            // draw_vertices,

        // math::rect bounding_rect = path.bounding_rect();
        // std::vector<math::rect> lines = math::lines_from_rect(bounding_rect);
        // geom::Path rect;
        // rect.move_to(lines[0].min);

        // for (auto& line : lines) {
        //   rect.line_to(line.max);
        // }

        // Renderer::Renderer::draw_outline(id, rect, position);
      }
    }

    // {
    //   std::vector<math::rect> lines = math::lines_from_rect(viewport.visible());
    //   geom::Path rect;
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

    renderer::Renderer::end_frame();

#if 0
    GK_DEBUGGER_RENDER(vec2(viewport.size()) * viewport.dpr());
#endif
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
    m_entities.erase(it);
    m_order.erase(std::remove(m_order.begin(), m_order.end(), entity), m_order.end());
    m_registry.destroy(entity);
  }

}
