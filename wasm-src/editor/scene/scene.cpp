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
 * @todo implement layer and groups history
 * @todo entity_at with open path not working (close the path)
 */

#include "scene.h"

#include "../../geom/intersections.h"
#include "../../geom/path.h"
#include "../../geom/path_builder.h"

#include "../../math/math.h"
#include "../../math/matrix.h"

#include "../../renderer/renderer.h"

#include "../../utils/debugger.h"
#include "../../utils/misc.h"

#include "../input/input_manager.h"
#include "../input/tools/pen_tool.h"

#include "../settings.h"

#include "entity.h"

namespace graphick::editor {

/**
 * @brief The type of hit test to perform.
 */
enum class HitTestType {
  All,         // Both fill, stroke and outline.
  EntityOnly,  // Only the fill and stroke, no outline.
  OutlineOnly  // Only the outline (used for selected entities priority).
};

static std::vector<vec4> s_layer_colors = {vec4(79, 128, 255, 255) / 255.0f,
                                           vec4(255, 79, 79, 255) / 255.0f,
                                           vec4(79, 255, 79, 255) / 255.0f};

Scene::Scene() : selection(this), history(this)
{
  Entity entity = {m_registry.create(), this};
  uuid id = uuid();

  entity.add<IDComponent>(id);
  entity.add<ArtboardComponent>(vec4(25, 29, 36, 255) / 255.0f);

  m_background = entity;
  m_entities[id] = entity;

  create_layer();

  history.clear();
}

Scene::Scene(const Scene& other)
    : m_entities(other.m_entities),
      m_background(other.m_background),
      m_layers(other.m_layers),
      selection(this),
      history(this),
      viewport(other.viewport)
{
}

Scene::Scene(Scene&& other) noexcept
    : m_registry(std::move(other.m_registry)),
      m_background(other.m_background),
      m_entities(std::move(other.m_entities)),
      selection(this),
      history(this)
{
}

Scene::~Scene() {}

bool Scene::has_entity(const uuid id) const
{
  return m_entities.find(id) != m_entities.end();
}

Entity Scene::get_entity(const uuid id)
{
  return {m_entities.at(id), this};
}

const Entity Scene::get_entity(const uuid id) const
{
  return {m_entities.at(id), const_cast<Scene*>(this)};
}

Entity Scene::get_active_layer()
{
  GK_ASSERT(!m_layers.empty(), "No layers in scene!");

  const size_t active_layer = m_active_layer >= m_layers.size() ? 0 : m_active_layer;

  GK_ASSERT(m_registry.all_of<LayerComponent::Data>(m_layers[active_layer]),
            "Active layer is not a layer!");

  return {m_layers[active_layer], this};
}

Entity Scene::get_background()
{
  return {m_background, this};
}

const Entity Scene::get_background() const
{
  return {m_background, const_cast<Scene*>(this)};
}

const Entity Scene::get_active_layer() const
{
  GK_ASSERT(!m_layers.empty(), "No layers in scene!");

  const size_t active_layer = m_active_layer >= m_layers.size() ? 0 : m_active_layer;

  GK_ASSERT(m_registry.all_of<LayerComponent::Data>(m_layers[active_layer]),
            "Active layer is not a layer!");

  return {m_layers[m_active_layer], const_cast<Scene*>(this)};
}

Entity Scene::create_layer()
{
  const uuid id = uuid();

  Entity entity = {m_registry.create(), this};

  entity.add<IDComponent>(id);
  entity.add<TagComponent>("Layer " + std::to_string(m_entity_tag_number++));
  entity.add<CategoryComponent>(CategoryComponent::Category::None);
  entity.add<LayerComponent>(s_layer_colors[m_layer_tag_number++ % s_layer_colors.size()]);

  history.add(id, Action::Target::Entity, std::move(entity.encode()), false);

  m_entities[id] = entity;
  m_layers.push_back(entity);

  m_active_layer = m_layers.size() - 1;

  return entity;
}

Entity Scene::create_group(const std::vector<entt::entity>& entities)
{
  const uuid id = uuid();

  Entity entity = create_entity(id, "Group");

  entity.add<GroupComponent>(entities);
  history.add(id, Action::Target::Entity, std::move(entity.encode()), false);

  return entity;
}

Entity Scene::create_element()
{
  const uuid id = uuid();

  Entity entity = create_entity(id, "Element");

  entity.add<PathComponent>();
  history.add(id, Action::Target::Entity, std::move(entity.encode()), false);

  return entity;
}

Entity Scene::create_element(const geom::path& path)
{
  const uuid id = uuid();

  Entity entity = create_entity(id, "Element");

  entity.add<PathComponent>(path);
  history.add(id, Action::Target::Entity, std::move(entity.encode()), false);

  return entity;
}

Entity Scene::create_image(const uuid image_id)
{
  const uuid id = uuid();

  Entity entity = create_entity(id, "Image");

  entity.add<ImageComponent>(image_id);
  history.add(id, Action::Target::Entity, std::move(entity.encode()), false);

  return entity;
}

Entity Scene::create_text(const std::string& text, const uuid font_id = uuid::null)
{
  const uuid id = uuid();

  Entity entity = create_entity(id, "Text");

  entity.add<TextComponent>(text, font_id);
  history.add(id, Action::Target::Entity, std::move(entity.encode()), false);

  return entity;
}

void Scene::delete_entity(Entity entity)
{
  history.remove(entity.id(), Action::Target::Entity, entity.encode());
}

void Scene::delete_entity(uuid id)
{
  delete_entity(get_entity(id));
}

uuid entity_at(const Entity entity,
               const mat2x3& parent_transform,
               const bool parent_selected,
               const vec2 position,
               const bool deep_search,
               const float threshold,
               const HitTestType parent_hit_test_type,
               const Cache* cache,
               Scene* scene)
{
  const uuid id = entity.id();
  const bool selected = parent_selected || scene->selection.has(id);
  const bool deep_search_entity = deep_search && selected;

  const HitTestType hit_test_type = (parent_hit_test_type == HitTestType::All && selected) ?
                                        HitTestType::EntityOnly :
                                        parent_hit_test_type;

  if (entity.is_element()) {
    if (!deep_search_entity && cache->renderer_cache.has_bounding_rect(id)) {
      const rect bounding_rect = rect(cache->renderer_cache.get_bounding_rect(id));

      if (!geom::is_point_in_rect(position, bounding_rect, threshold)) {
        return uuid::null;
      }
    }

    const geom::path& path = entity.get_component<PathComponent>().data();
    const mat2x3& transform = entity.get_component<TransformComponent>();

    bool has_fill = false;
    bool has_stroke = false;

    geom::FillingOptions filling_options;
    geom::StrokingOptions<float> stroking_options;

    if (hit_test_type != HitTestType::OutlineOnly && entity.has_component<FillComponent>()) {
      auto& fill_component = entity.get_component<FillComponent>();

      filling_options = geom::FillingOptions{fill_component.rule()};
      has_fill = fill_component.visible() && fill_component.paint().visible();
    }

    if (entity.has_component<StrokeComponent>()) {
      auto& stroke_component = entity.get_component<StrokeComponent>();

      stroking_options = geom::StrokingOptions<float>{
          static_cast<float>(Settings::Renderer::stroking_tolerance),
          stroke_component.width(),
          stroke_component.miter_limit(),
          stroke_component.cap(),
          stroke_component.join()};
      has_stroke = stroke_component.visible() && stroke_component.paint().visible();
    }

    if (hit_test_type == HitTestType::OutlineOnly ||
        (!has_stroke && hit_test_type == HitTestType::All))
    {
      stroking_options = geom::StrokingOptions<float>{
          static_cast<float>(Settings::Renderer::stroking_tolerance),
          0.0f,
          0.0f,
          geom::LineCap::Square,
          geom::LineJoin::Bevel};
      has_stroke = true;
    }

    return path.is_point_inside_path(position,
                                     has_fill ? &filling_options : nullptr,
                                     has_stroke ? &stroking_options : nullptr,
                                     parent_transform * transform,
                                     threshold,
                                     deep_search_entity) ?
               id :
               uuid::null;
  } else if (entity.is_image()) {
    const mat2x3& transform = entity.get_component<TransformComponent>();
    const ImageComponent image = entity.get_component<ImageComponent>();
    const geom::path path = image.path();

    const geom::FillingOptions filling_options{geom::FillRule::NonZero};
    const geom::StrokingOptions<float> stroking_options{
        static_cast<float>(Settings::Renderer::stroking_tolerance),
        0.0f,
        0.0f,
        geom::LineCap::Square,
        geom::LineJoin::Bevel};

    return path.is_point_inside_path(
               position,
               hit_test_type != HitTestType::OutlineOnly ? &filling_options : nullptr,
               hit_test_type != HitTestType::EntityOnly ? &stroking_options : nullptr,
               parent_transform * transform,
               threshold,
               false) ?
               id :
               uuid::null;
  } else if (entity.is_text()) {
    return uuid::null;
  } else if (entity.is_group()) {
    const GroupComponent group = entity.get_component<GroupComponent>();
    const mat2x3& transform = entity.get_component<TransformComponent>();

    for (auto it = group.rbegin(); it != group.rend(); it++) {
      const Entity entity = {*it, scene};

      const uuid child_id = editor::entity_at(entity,
                                              parent_transform * transform,
                                              parent_selected,
                                              position,
                                              deep_search_entity,
                                              threshold,
                                              hit_test_type,
                                              cache,
                                              scene);

      if (child_id != uuid::null) {
        // If not deep searching, return the entire group.
        return deep_search_entity ? child_id : id;
      }
    }
  } else if (entity.is_layer()) {
    const LayerComponent layer = entity.get_component<LayerComponent>();

    for (auto it = layer.rbegin(); it != layer.rend(); it++) {
      const Entity entity = {*it, scene};

      const uuid child_id = editor::entity_at(entity,
                                              parent_transform,
                                              parent_selected,
                                              position,
                                              deep_search_entity,
                                              threshold,
                                              hit_test_type,
                                              cache,
                                              scene);

      if (child_id != uuid::null) {
        // An entire layer cannot be selected.
        return child_id;
      }
    }
  }

  return uuid::null;
}

uuid Scene::entity_at(const vec2 position, const bool deep_search, const float threshold) const
{
  __debug_time_total();

  const float zoom = viewport.zoom();
  const float local_threshold = threshold / zoom;

  for (const auto& [id, entry] : selection.selected()) {
    // TODO: fix this identity transform
    if (editor::entity_at(get_entity(id),
                          mat2x3::identity(),
                          true,
                          position,
                          deep_search,
                          local_threshold,
                          HitTestType::OutlineOnly,
                          &m_cache,
                          const_cast<Scene*>(this)) != uuid::null)
    {
      return id;
    }
  }

  for (auto it = m_layers.rbegin(); it != m_layers.rend(); it++) {
    const Entity layer = {*it, const_cast<Scene*>(this)};

    const uuid id = editor::entity_at(layer,
                                      mat2x3::identity(),
                                      false,
                                      position,
                                      deep_search,
                                      local_threshold,
                                      HitTestType::All,
                                      &m_cache,
                                      const_cast<Scene*>(this));

    if (id != uuid::null) {
      return id;
    }
  }

  return uuid::null;
}

Entity Scene::duplicate_entity(const uuid id)
{
  Entity entity = get_entity(id);

  auto [new_id, data] = entity.duplicate();

  history.add(new_id, Action::Target::Entity, std::move(data));

  return get_entity(new_id);
}

static void entities_in(const Entity entity,
                        const mat2x3 parent_transform,
                        const math::rect& rect,
                        const bool deep_search,
                        std::unordered_map<uuid, Selection::SelectionEntry>& entities,
                        std::vector<uint32_t>& vertices,
                        std::vector<uuid>& group_hierarchy)
{
  if (!entity.has_component<TransformComponent>()) {
    return;
  }

  const uuid id = entity.id();
  const uuid entry_id = deep_search || group_hierarchy.empty() ? id : group_hierarchy.back();

  if (entity.is_group()) {
    const GroupComponent group = entity.get_component<GroupComponent>();

    group_hierarchy.push_back(id);

    for (auto it = group.rbegin(); it != group.rend(); it++) {
      const Entity entity = {*it, entity.scene()};
      const mat2x3& transform = entity.get_component<TransformComponent>();

      editor::entities_in(entity,
                          parent_transform * transform,
                          rect,
                          deep_search,
                          entities,
                          vertices,
                          group_hierarchy);
    }

    group_hierarchy.pop_back();
  } else if (entity.is_layer()) {
    const LayerComponent layer = entity.get_component<LayerComponent>();

    for (auto it = layer.rbegin(); it != layer.rend(); it++) {
      const Entity entity = {*it, entity.scene()};

      editor::entities_in(
          entity, parent_transform, rect, deep_search, entities, vertices, group_hierarchy);
    }
  } else {
    const TransformComponent transform = entity.get_component<TransformComponent>();
    const mat2x3 total_transform = parent_transform * transform.matrix();
    const float angle = math::rotation(total_transform);

    if (math::is_almost_zero(std::fmodf(angle, math::two_pi<float>))) {
      const mat2x3 inverse_transform = math::inverse(total_transform);
      const math::rect selection_rect = inverse_transform * rect;

      if (entity.is_element()) {
        if (geom::is_rect_in_rect(transform.bounding_rect(), rect)) {
          entities.insert({entry_id, Selection::SelectionEntry()});
          return;
        }

        const PathComponent path = entity.get_component<PathComponent>();

        if (deep_search) {
          vertices.clear();

          if (path.data().intersects(selection_rect, &vertices)) {
            entities.insert({entry_id,
                             Selection::SelectionEntry{
                                 std::unordered_set<uint32_t>(vertices.begin(), vertices.end())}});
          }
        } else {
          if (path.data().intersects(selection_rect)) {
            entities.insert({entry_id, Selection::SelectionEntry()});
          }
        }
      } else {
        if (geom::does_rect_intersect_rect(transform.bounding_rect(), rect)) {
          entities.insert({entry_id, Selection::SelectionEntry()});
        }
      }
    } else {
      if (entity.is_element()) {
        if (geom::is_rect_in_rect(transform.bounding_rect(), rect)) {
          entities.insert({entry_id, Selection::SelectionEntry()});
          return;
        }

        const PathComponent path = entity.get_component<PathComponent>();

        if (deep_search) {
          vertices.clear();

          if (path.data().intersects(rect, transform, &vertices)) {
            entities.insert({entry_id,
                             Selection::SelectionEntry{
                                 std::unordered_set<uint32_t>(vertices.begin(), vertices.end())}});
          }
        } else {
          if (path.data().intersects(rect, transform)) {
            entities.insert({entry_id, Selection::SelectionEntry()});
          }
        }
      } else if (entity.is_image()) {
        if (geom::is_rect_in_rect(transform.bounding_rect(), rect)) {
          entities.insert({entry_id, Selection::SelectionEntry()});
          return;
        }

        const ImageComponent image = entity.get_component<ImageComponent>();
        const geom::path path = image.path();

        if (path.intersects(rect, transform)) {
          entities.insert({entry_id, Selection::SelectionEntry()});
        }
      } else {
        if (geom::does_rect_intersect_rect(transform.bounding_rect(), rect)) {
          entities.insert({entry_id, Selection::SelectionEntry()});
        }
      }
    }
  }
}

std::unordered_map<uuid, Selection::SelectionEntry> Scene::entities_in(const math::rect& rect,
                                                                       bool deep_search)
{
  std::unordered_map<uuid, Selection::SelectionEntry> entities;
  std::vector<uint32_t> vertices;
  std::vector<uuid> group_hierarchy;

  for (auto it = m_layers.begin(); it != m_layers.end(); it++) {
    const Entity layer = {*it, const_cast<Scene*>(this)};

    group_hierarchy.clear();

    editor::entities_in(
        layer, mat2x3::identity(), rect, deep_search, entities, vertices, group_hierarchy);
  }

  return entities;

  auto view = get_all_entities_with<IDComponent::Data, TransformComponent::Data>();

  for (entt::entity handle : view) {
    const Entity entity = {handle, this};
    const uuid id = entity.id();
    const TransformComponent transform = entity.get_component<TransformComponent>();

    const float angle = math::rotation(transform.matrix());

    if (math::is_almost_zero(std::fmodf(angle, math::two_pi<float>))) {
      const mat2x3 inverse_transform = transform.inverse();
      const math::rect selection_rect = inverse_transform * rect;

      if (entity.is_element()) {
        if (geom::is_rect_in_rect(transform.bounding_rect(), rect)) {
          entities.insert({id, Selection::SelectionEntry()});
          continue;
        }

        const PathComponent path = entity.get_component<PathComponent>();

        if (deep_search) {
          vertices.clear();

          if (path.data().intersects(selection_rect, &vertices)) {
            entities.insert({id,
                             Selection::SelectionEntry{
                                 std::unordered_set<uint32_t>(vertices.begin(), vertices.end())}});
          }
        } else {
          if (path.data().intersects(selection_rect)) {
            entities.insert({id, Selection::SelectionEntry()});
          }
        }
      } else {
        if (geom::does_rect_intersect_rect(transform.bounding_rect(), rect)) {
          entities.insert({id, Selection::SelectionEntry()});
        }
      }
    } else {
      if (entity.is_element()) {
        if (geom::is_rect_in_rect(transform.bounding_rect(), rect)) {
          entities.insert({id, Selection::SelectionEntry()});
          continue;
        }

        const PathComponent path = entity.get_component<PathComponent>();

        if (deep_search) {
          vertices.clear();

          if (path.data().intersects(rect, transform, &vertices)) {
            entities.insert({id,
                             Selection::SelectionEntry{
                                 std::unordered_set<uint32_t>(vertices.begin(), vertices.end())}});
          }
        } else {
          if (path.data().intersects(rect, transform)) {
            entities.insert({id, Selection::SelectionEntry()});
          }
        }
      } else if (entity.is_image()) {
        if (geom::is_rect_in_rect(transform.bounding_rect(), rect)) {
          entities.insert({id, Selection::SelectionEntry()});
          continue;
        }

        const ImageComponent image = entity.get_component<ImageComponent>();
        const geom::path path = image.path();

        if (path.intersects(rect, transform)) {
          entities.insert({id, Selection::SelectionEntry()});
        }
      } else {
        if (geom::does_rect_intersect_rect(transform.bounding_rect(), rect)) {
          entities.insert({id, Selection::SelectionEntry()});
        }
      }
    }
  }

  return entities;
}

void Scene::group_selected()
{
  // TODO: handle multiple layers
  // TODO: history
  // TODO: z-ordering

  LayerComponent layer = get_active_layer().get_component<LayerComponent>();

  std::vector<entt::entity> entities;

  for (const auto& [id, entry] : selection.selected()) {
    Entity entity = get_entity(id);

    entities.push_back(entity);
    layer.remove(entity);
  }

  Entity group = create_group(entities);

  selection.clear();
  selection.select(group.id());

  console::log("group_selected");
}

void Scene::ungroup_selected()
{
  console::log("ungroup_selected");
}

static void render_element(const Entity& entity,
                           const uuid id,
                           const mat2x3& parent_transform,
                           const bool selected,
                           const bool temp_selected,
                           const entt::registry* registry,
                           const renderer::Outline* parent_outline,
                           const Scene* scene)
{
  const bool has_outline = selected || temp_selected;

  renderer::Fill fill_opt;
  renderer::Stroke stroke_opt;
  renderer::Outline outline_opt;
  renderer::DrawingOptions options;

  bool has_fill = registry->all_of<FillData>(entity);
  bool has_stroke = registry->all_of<StrokeData>(entity);

  if (has_fill) {
    const FillData& fill = registry->get<FillData>(entity);

    if (fill.visible && fill.paint.visible()) {
      fill_opt = renderer::Fill(fill);
      options.fill = &fill_opt;
    } else {
      has_fill = false;
    }
  }

  if (has_stroke) {
    const StrokeData& stroke = registry->get<StrokeData>(entity);

    if (stroke.visible && stroke.paint.visible()) {
      stroke_opt = renderer::Stroke(stroke);
      options.stroke = &stroke_opt;
    } else {
      has_stroke = false;
    }
  }

  if (!has_fill && !has_stroke && !has_outline) {
    return;
  }

  if (has_outline) {
    outline_opt = *parent_outline;
    outline_opt.draw_vertices = parent_outline->draw_vertices && has_outline;

    options.outline = &outline_opt;
  }

  const geom::path& path_data = registry->get<PathData>(entity).path;
  const mat2x3& transform = registry->get<TransformData>(entity).matrix;

  const mat2x3 total_transform = parent_transform * transform;

  if (!has_outline || !outline_opt.draw_vertices) {
    renderer::Renderer::draw(path_data, total_transform, options, id);
    return;
  }

  std::unordered_set<uint32_t> selected_vertices;
  bool is_full = false;

  if (selected) {
    if (scene->selection.selected().find(id) != scene->selection.selected().end()) {
      const Selection::SelectionEntry& entry = scene->selection.selected().at(id);

      is_full = entry.full();

      if (!is_full) {
        selected_vertices = entry.indices;
      }
    } else {
      is_full = true;
    }
  }

  if (temp_selected && !is_full) {
    if (scene->selection.temp_selected().find(id) != scene->selection.temp_selected().end()) {
      const Selection::SelectionEntry& entry = scene->selection.temp_selected().at(id);

      is_full = entry.full();

      if (!is_full) {
        selected_vertices.insert(scene->selection.temp_selected().at(id).indices.begin(),
                                 scene->selection.temp_selected().at(id).indices.end());
      }
    } else {
      is_full = true;
    }
  }

  outline_opt.selected_vertices = is_full ? nullptr : &selected_vertices;

  renderer::Renderer::draw(path_data, total_transform, options, id);
}

static void render_image(const Entity& entity,
                         const uuid id,
                         const mat2x3& parent_transform,
                         const bool selected,
                         const bool temp_selected,
                         const entt::registry* registry,
                         const renderer::Outline* parent_outline,
                         const Scene* scene)
{
  const bool has_outline = selected || temp_selected;

  renderer::Fill fill_opt;
  renderer::Stroke stroke_opt;
  renderer::DrawingOptions options;

  bool has_stroke = registry->all_of<StrokeData>(entity);

  options.fill = &fill_opt;

  if (has_stroke) {
    const StrokeData& stroke = registry->get<StrokeData>(entity);

    if (stroke.visible && stroke.paint.visible()) {
      stroke_opt = renderer::Stroke(stroke);
      options.stroke = &stroke_opt;
    } else {
      has_stroke = false;
    }
  }

  const ImageComponent image = entity.get_component<ImageComponent>();
  const mat2x3& transform = registry->get<TransformData>(entity).matrix;

  const mat2x3 total_transform = parent_transform * transform;
  const vec2 size = vec2(image.size());

  fill_opt = renderer::Fill(
      image.id(), renderer::Paint::Type::TexturePaint, renderer::FillRule::NonZero);

  if (has_outline) {
    options.outline = parent_outline;
  }

  renderer::Renderer::draw(image.path(), total_transform, options, id);
}

static void render_entity(const Entity& entity,
                          const mat2x3& parent_transform,
                          const bool parent_selected,
                          const bool parent_temp_selected,
                          const entt::registry* registry,
                          renderer::Outline* outline,
                          Scene* scene)
{
  const uuid id = entity.id();

  const bool selected = parent_selected ||
                        scene->selection.selected().find(id) != scene->selection.selected().end();
  const bool temp_selected = parent_temp_selected || scene->selection.temp_selected().find(id) !=
                                                         scene->selection.temp_selected().end();

  if (entity.is_element()) {
    render_element(
        entity, id, parent_transform, selected, temp_selected, registry, outline, scene);
  } else if (entity.is_image()) {
    render_image(entity, id, parent_transform, selected, temp_selected, registry, outline, scene);
  } else if (entity.is_text()) {
  } else if (entity.is_group()) {
    const GroupComponent group = entity.get_component<GroupComponent>();
    const mat2x3& transform = entity.get_component<TransformComponent>();

    for (auto it = group.begin(); it != group.end(); it++) {
      const Entity entity = {*it, scene};

      render_entity(
          entity, parent_transform * transform, selected, temp_selected, registry, outline, scene);
    }
  } else if (entity.is_layer()) {
    const LayerComponent layer = entity.get_component<LayerComponent>();

    outline->color = layer.color();

    for (auto it = layer.begin(); it != layer.end(); it++) {
      const Entity entity = {*it, scene};

      render_entity(entity, parent_transform, selected, temp_selected, registry, outline, scene);
    }
  }
}

void Scene::render(const bool ignore_cache) const
{
  __debug_time_total();

  const double dpr = static_cast<double>(viewport.dpr());
  const double zoom = static_cast<double>(viewport.zoom()) * dpr;

  // Flooring to avoid banding artifacts.
  const ivec2 viewport_size = ivec2(math::floor(dvec2(viewport.size()) * dpr));
  const dvec2 viewport_position = dvec2(viewport.position());
  const vec4 background = get_background().get_component<ArtboardComponent>().color();

  const renderer::Viewport rendering_viewport = {
      viewport_size, viewport_position, zoom, dpr, background};

  renderer::Renderer::begin_frame({rendering_viewport, &m_cache.renderer_cache, ignore_cache});

  const auto& selected = selection.selected();
  const auto& temp_selected = selection.temp_selected();
  const bool draw_vertices = tool_state.active().is_in_category(input::Tool::CategoryDirect);

  renderer::Outline outline = {nullptr, draw_vertices, Settings::Renderer::ui_primary_color};

  for (auto it = m_layers.begin(); it != m_layers.end(); it++) {
    const Entity layer = {*it, const_cast<Scene*>(this)};

    render_entity(
        layer, mat2x3::identity(), false, false, &m_registry, &outline, const_cast<Scene*>(this));
  }

  tool_state.render_overlays(get_active_layer().get_component<LayerComponent>().color(),
                             viewport.zoom());

  renderer::Renderer::end_frame();
}

Entity Scene::create_entity(const uuid id, const std::string& tag_type)
{
  LayerComponent layer = get_active_layer().get_component<LayerComponent>();
  Entity entity = {m_registry.create(), this};

  entity.add<IDComponent>(id);
  entity.add<TagComponent>(tag_type + " " + std::to_string(m_entity_tag_number++));
  entity.add<CategoryComponent>(CategoryComponent::Category::Selectable);
  entity.add<TransformComponent>();

  m_entities[id] = entity;

  layer.push_back(entity);

  return entity;
}

void Scene::add(const uuid id, const io::EncodedData& encoded_data)
{
  LayerComponent layer = get_active_layer().get_component<LayerComponent>();
  Entity entity = {m_registry.create(), this, encoded_data};

  m_entities[id] = entity;

  layer.push_back(entity);
}

void Scene::remove(const uuid id)
{
  auto it = m_entities.find(id);
  if (it == m_entities.end()) {
    return;
  }

  entt::entity entity = it->second;

  selection.deselect(id);
  m_entities.erase(it);

  bool is_layer = false;

  for (auto it = m_layers.begin(); it != m_layers.end(); it++) {
    const Entity layer_entity = {*it, const_cast<Scene*>(this)};

    if (*it == entity) {
      is_layer = true;
      break;
    } else if (!layer_entity.is_layer()) {
      continue;
    }

    LayerComponent layer = layer_entity.get_component<LayerComponent>();

    layer.remove(entity);
  }

  m_layers.erase(std::remove(m_layers.begin(), m_layers.end(), entity), m_layers.end());

  m_registry.destroy(entity);
}

}  // namespace graphick::editor
