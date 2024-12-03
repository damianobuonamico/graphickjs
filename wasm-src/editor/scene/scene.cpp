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

#include "../../geom/intersections.h"
#include "../../geom/path.h"
#include "../../geom/path_builder.h"

#include "../../math/math.h"
#include "../../math/matrix.h"

#include "../../renderer/renderer.h"

#include "../../utils/console.h"
#include "../../utils/misc.h"

#include "../input/input_manager.h"
#include "../input/tools/pen_tool.h"

#include "../settings.h"

#include "entity.h"

namespace graphick::editor {

Scene::Scene() : selection(this), history(this) {}

Scene::Scene(const Scene& other)
    : m_entities(other.m_entities),
      m_order(other.m_order),
      selection(this),
      history(this),
      viewport(other.viewport)
{
}

Scene::Scene(Scene&& other) noexcept
    : m_registry(std::move(other.m_registry)),
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

uuid Scene::entity_at(const vec2 position, const bool deep_search, const float threshold) const
{
  GK_TOTAL("Scene::entity_at");

  const float zoom = viewport.zoom();
  const float local_threshold = threshold / zoom;

  for (const auto& [id, entry] : selection.selected()) {
    if (is_entity_at(
            get_entity(id), position, deep_search, local_threshold, HitTestType::OutlineOnly))
    {
      return id;
    }
  }

  for (auto it = m_order.rbegin(); it != m_order.rend(); it++) {
    const Entity entity = {*it, const_cast<Scene*>(this)};
    const HitTestType hit_test_type = selection.has(entity.id()) ? HitTestType::EntityOnly :
                                                                   HitTestType::All;

    if (is_entity_at(entity, position, deep_search, local_threshold, hit_test_type)) {
      return entity.id();
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

// TODO: transformed images are not being tested correctly
std::unordered_map<uuid, Selection::SelectionEntry> Scene::entities_in(const math::rect& rect,
                                                                       bool deep_search)
{
  std::unordered_map<uuid, Selection::SelectionEntry> entities;
  std::vector<uint32_t> vertices;

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
      } else {
        if (geom::does_rect_intersect_rect(transform.bounding_rect(), rect)) {
          entities.insert({id, Selection::SelectionEntry()});
        }
      }
    }
  }

  return entities;
}

void Scene::render(const bool ignore_cache) const
{
  GK_TOTAL("Scene::render");

  const math::rect visible_rect = viewport.visible();
  const renderer::Viewport rendering_viewport = {
      ivec2(math::floor(vec2(viewport.size()) *
                        viewport.dpr())),  // Flooring to avoid banding artifacts.
      dvec2(viewport.position()),
      double(viewport.zoom() * viewport.dpr()),
      double(viewport.dpr()),
      vec4{0.2f, 0.2f, 0.21f, 1.0f}};

  renderer::Renderer::begin_frame({rendering_viewport, &m_cache, ignore_cache});

  uint32_t z_index = 1;

  bool should_rehydrate = true;

  auto& selected = selection.selected();
  auto& temp_selected = selection.temp_selected();

  bool draw_vertices = tool_state.active().is_in_category(input::Tool::CategoryDirect);

  // TODO: handle objects with transparency
  for (auto it = m_order.begin(); it != m_order.end(); it++) {
    const Entity entity = {*it, const_cast<Scene*>(this)};
    const uuid id = entity.id();
    const TransformComponent transform = entity.get_component<TransformComponent>();

    if (!has_entity(id)) {
      return;
    }

    const bool is_selected = selected.find(id) != selected.end();
    const bool is_temp_selected = temp_selected.find(id) != temp_selected.end();
    const bool has_outline = is_selected || is_temp_selected;

    // TODO: this is temp, every tipe of entity should be extracted in a separate method
    if (entity.is_image()) {
      const ImageComponent image = entity.get_component<ImageComponent>();
      const vec2 size = vec2(image.size());

      renderer::Fill fill{
          image.id(), renderer::Paint::Type::TexturePaint, renderer::FillRule::NonZero};

      renderer::Outline outline{nullptr, false, Settings::Renderer::ui_primary_color};

      // TODO: if possible, should render as 4 tiles and 1 fill (not 1 fill due to subpixel
      // positioning)
      renderer::Renderer::draw(image.path(),
                               transform.matrix(),
                               {&fill, nullptr, has_outline ? &outline : nullptr},
                               id);

      continue;
    } else if (entity.is_text()) {
      const TextComponent text = entity.get_component<TextComponent>();

      // TODO: in the TextComponent there should be an array of fills and strokes for each
      // character range.
      // TODO: there should be a uuid set in addition to the cache map: when we query the cache, we
      // should remove the uuid from the set. At the end of the frame, we remove all the uuids that
      // are still in the set from the cache.
      renderer::Renderer::draw(text, transform.matrix(), vec4(0.9f));

      continue;
    }

    const geom::path& path = entity.get_component<PathComponent>().data();

    if (should_rehydrate) {
      // path.rehydrate_cache();
    }

    bool has_fill = m_registry.all_of<FillComponent::Data>(*it);
    bool has_stroke = m_registry.all_of<StrokeComponent::Data>(*it);

    const std::optional<FillComponent::Data> fill =
        has_fill ? std::optional<FillComponent::Data>(m_registry.get<FillComponent::Data>(*it)) :
                   std::nullopt;
    const std::optional<StrokeComponent::Data> stroke =
        has_stroke ?
            std::optional<StrokeComponent::Data>(m_registry.get<StrokeComponent::Data>(*it)) :
            std::nullopt;

    if (has_fill && !fill->visible) {
      has_fill = false;
    }
    if (has_stroke && !stroke->visible) {
      has_stroke = false;
    }

    if (!has_fill && !has_stroke && !has_outline) {
      continue;
    }

    const mat2x3& transform_matrix = transform.matrix();
    std::unordered_set<uint32_t> selected_vertices;

    renderer::Fill fill_opt;
    renderer::Stroke stroke_opt;
    renderer::Outline outline_opt;

    if (has_fill) {
      fill_opt = renderer::Fill(fill->paint, fill->rule);
    }

    if (has_stroke) {
      stroke_opt = renderer::Stroke(
          stroke->paint, stroke->cap, stroke->join, stroke->width, stroke->miter_limit);
    }

    if (has_outline) {
      bool is_full = false;

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
          selected_vertices.insert(temp_selected.at(id).indices.begin(),
                                   temp_selected.at(id).indices.end());
        }
      }

      outline_opt = renderer::Outline{is_full ? nullptr : &selected_vertices,
                                      draw_vertices,
                                      Settings::Renderer::ui_primary_color};
    }

    renderer::Renderer::draw(path,
                             transform_matrix,
                             renderer::DrawingOptions{has_fill ? &fill_opt : nullptr,
                                                      has_stroke ? &stroke_opt : nullptr,
                                                      has_outline ? &outline_opt : nullptr},
                             m_registry.get<IDComponent::Data>(*it).id);
  }

  // {
  //   std::vector<math::rect> lines =
  //   math::lines_from_rect(viewport.visible()); geom::path rect;
  //   rect.move_to(lines[0].min);

  //   for (auto& line : lines) {
  //     rect.line_to(line.max);
  //   }

  //   Renderer::Renderer::draw_outline(0, rect, { 0.0f, 0.0f });
  // }

  {
    tool_state.render_overlays(viewport.zoom());
  }

  // #ifdef GK_DEBUG
  //     if (!selected.empty()) {
  //       const Entity entity = { m_entities.at(selected.begin()->first),
  //       const_cast<Scene*>(this) };

  //       if (entity.has_component<PathComponent>()) {
  //         const geom::path& path =
  //         entity.get_component<PathComponent>().data();

  //         if (!path.empty()) {
  //           const geom::path::Segment segment = path.segment_at(0);
  //           renderer::Renderer::draw_debug_overlays({ segment.p0, segment.p1,
  //           segment.p2, segment.p3 });
  //         }
  //       }
  //     }
  // #endif

  renderer::Renderer::end_frame();

  m_cache.set_grid_rect(viewport.visible(), ivec2(viewport.size() / 128.0f));

#if 0
    GK_DEBUGGER_RENDER(vec2(viewport.size()) * viewport.dpr());
#endif
}

Entity Scene::create_entity(const uuid id, const std::string& tag_type)
{
  Entity entity = {m_registry.create(), this};

  entity.add<IDComponent>(id);
  entity.add<TagComponent>(tag_type + " " + std::to_string(m_entity_tag_number++));
  entity.add<CategoryComponent>(CategoryComponent::Category::Selectable);
  entity.add<TransformComponent>();

  m_entities[id] = entity;
  m_order.push_back(entity);

  return entity;
}

void Scene::add(const uuid id, const io::EncodedData& encoded_data)
{
  Entity entity = {m_registry.create(), this, encoded_data};

  m_entities[id] = entity;
  m_order.push_back(entity);
}

void Scene::remove(const uuid id)
{
  auto it = m_entities.find(id);
  if (it == m_entities.end())
    return;

  entt::entity entity = it->second;

  selection.deselect(id);
  m_entities.erase(it);
  m_order.erase(std::remove(m_order.begin(), m_order.end(), entity), m_order.end());
  m_registry.destroy(entity);
}

bool Scene::is_entity_at(const Entity entity,
                         const vec2 position,
                         const bool deep_search,
                         const float threshold,
                         const HitTestType hit_test_type) const
{
  const uuid id = entity.id();
  const bool is_element = entity.is_element();

  const bool deep_search_entity = deep_search && selection.has(id);

  if ((!deep_search_entity || !is_element) && m_cache.renderer_cache.has_bounding_rect(id)) {
    const rect bounding_rect = rect(m_cache.renderer_cache.get_bounding_rect(id));

    if (!geom::is_point_in_rect(position, bounding_rect, threshold)) {
      return false;
    }
  }

  const TransformComponent transform = entity.get_component<TransformComponent>();

  if (!is_element) {
    // TODO: what to do here?
    if (entity.is_image()) {
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
          transform,
          threshold,
          deep_search_entity);
    }
    return geom::is_point_in_rect(position, transform.bounding_rect(), threshold);
  }

  const PathComponent& path_component = entity.get_component<PathComponent>();

  const geom::path& path = path_component.data();

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

  // TODO: check why path is empty when duplicated...
  return path.is_point_inside_path(position,
                                   has_fill ? &filling_options : nullptr,
                                   has_stroke ? &stroking_options : nullptr,
                                   transform,
                                   threshold,
                                   deep_search_entity);
}

}  // namespace graphick::editor
