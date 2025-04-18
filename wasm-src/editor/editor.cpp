/**
 * @file editor.cpp
 * @brief Contains the main Graphick Editor class implementation.
 *
 * @todo fix all todos (one day)
 * @todo editor reinitialization
 */

#include "editor.h"

#include "input/input_manager.h"
#include "scene/entity.h"
#include "scene/scene.h"

#include "../io/json/json.h"
#include "../io/resource_manager.h"

#include "../math/matrix.h"

#include "../renderer/renderer.h"

#include "../utils/console.h"

#ifdef EMSCRIPTEN
#  include <emscripten/html5.h>
#endif

namespace graphick::editor {

#ifdef EMSCRIPTEN
bool render_callback(const double time, void* user_data)
{
  Editor::get()->render_frame(time);
  return true;
}

EM_JS(void, msgbus_send, (const int msg_id), { window.msgbus.send(msg_id); });
#else
bool render_callback(const double time, void* user_data)
{
  return Editor::get()->render_frame(time);
}
#endif

#define MODIFY_SELECTED(component, ...) \
  for (const auto& [id, _] : get()->scene().selection.selected()) { \
    Entity entity = get()->scene().get_entity(id); \
    if (entity.has_component<component>()) { \
      entity.get_component<component>().__VA_ARGS__; \
    } \
  }

Editor* Editor::s_instance = nullptr;

void Editor::init()
{
  if (s_instance != nullptr) {
    console::error("Editor already initialized, call shutdown() before reinitializing!");
    return;
  }

  s_instance = new Editor();

  input::InputManager::init();
  io::ResourceManager::init();
  renderer::Renderer::init();

  s_instance->m_scenes.emplace_back();

#ifdef EMSCRIPTEN
  emscripten_request_animation_frame_loop(render_callback, nullptr);
#endif
}

void Editor::prepare_refresh()
{
  renderer::Renderer::shutdown();
}

void Editor::refresh()
{
  renderer::Renderer::init();
}

void Editor::shutdown()
{
  if (s_instance == nullptr) {
    console::error("Renderer already shutdown, call init() before shutting down!");
    return;
  }

  renderer::Renderer::shutdown();
  io::ResourceManager::shutdown();
  input::InputManager::shutdown();

  delete s_instance;
  s_instance = nullptr;
}

Scene& Editor::scene()
{
  if (get()->m_scenes.empty()) {
    get()->m_scenes.emplace_back();
  }

  return get()->m_scenes[0];
}

void Editor::resize(const ivec2 size, const ivec2 offset, float dpr)
{
  console::log(ui_data());

  for (auto& scene : get()->m_scenes) {
    scene.viewport.resize(size, offset, dpr);
  }
}

#ifndef EMSCRIPTEN
bool Editor::render_loop(const double time)
{
  return render_callback(time, nullptr);
}
#endif

void Editor::request_render(const RenderRequestOptions options)
{
  if (!get()->m_render_request.has_value()) {
    get()->m_render_request = options;
    return;
  }

  get()->m_render_request->update(options);
}

std::string Editor::ui_data()
{
  const Scene& scene = get()->scene();

  io::json::JSON data = io::json::JSON::object();
  io::json::JSON& components = data["components"] = io::json::JSON::object();

  if (scene.selection.size()) {
    const rrect selection_rrect = scene.selection.bounding_rrect();
    const rect selection_rect = rrect::to_rect(selection_rrect);

    const float selection_angle = selection_rrect.angle;
    const vec2 selection_size = selection_rrect.size();
    const vec2 selection_center = selection_rect.center();

    io::json::JSON& selection = components["transform"] = io::json::JSON::object();

    selection["x"] = selection_center.x;
    selection["y"] = selection_center.y;
    selection["w"] = selection_size.x;
    selection["h"] = selection_size.y;
    selection["angle"] = math::radians_to_degrees(selection_angle);

    for (const auto& [id, _] : scene.selection.selected()) {
      const Entity& entity = scene.get_entity(id);

      if (entity.has_component<FillComponent>()) {
        entity.get_component<FillComponent>().ui_data(components);
      }

      if (entity.has_component<StrokeComponent>()) {
        entity.get_component<StrokeComponent>().ui_data(components);
      }
    }
  } else {
    ArtboardComponent scene_background = scene.get_background().get_component<ArtboardComponent>();
    io::json::JSON& background = components["background"] = io::json::JSON::object();

    background["color"] = scene_background.color();
  }

  return data.dump();
}

void Editor::modify_ui_data(const std::string& data)
{
  io::json::JSON json = io::json::JSON::parse(data);

  if (!json.has("components")) {
    return;
  }

  io::json::JSON& components = json["components"];
  Scene& scene = get()->scene();

  if (components.has("background")) {
    io::json::JSON& background = components["background"];

    // TODO: check if color exists
    const vec4 color = background["color"].to_vec4();

    scene.get_background().get_component<ArtboardComponent>().color(color);
  }

  if (components.has("transform")) {
    io::json::JSON& transform = components["transform"];

    const rrect selection_rrect = scene.selection.bounding_rrect();
    const rect selection_rect = rrect::to_rect(selection_rrect);

    const float selection_angle = selection_rrect.angle;
    const vec2 selection_size = selection_rrect.size();
    const vec2 selection_center = selection_rect.center();
    const vec2 scale_center = selection_rrect.center();

    const vec2 center = {transform.has("x") ? transform["x"].to_float() : selection_center.x,
                         transform.has("y") ? transform["y"].to_float() : selection_center.y};
    const vec2 size = {transform.has("w") ? transform["w"].to_float() : selection_size.x,
                       transform.has("h") ? transform["h"].to_float() : selection_size.y};
    const float angle = transform.has("angle") ?
                            math::degrees_to_radians(transform["angle"].to_float()) :
                            selection_angle;

    const vec2 offset = center - selection_center;
    const vec2 scale = size / selection_size;

    for (auto& [id, _] : scene.selection.selected()) {
      Entity entity = scene.get_entity(id);

      if (entity.has_component<TransformComponent>()) {
        TransformComponent transform = entity.get_component<TransformComponent>();
        mat2x3 matrix = transform.matrix();

        if (!math::is_almost_equal(scale, vec2::one())) {
          matrix = math::rotate(math::scale(math::rotate(matrix, vec2::zero(), -selection_angle),
                                            scale_center,
                                            scale),
                                vec2::zero(),
                                selection_angle);
        } else if (!math::is_almost_equal(angle, selection_angle)) {
          matrix = math::rotate(matrix, selection_center, angle - selection_angle);
        } else if (!math::is_almost_zero(offset)) {
          matrix = math::translate(matrix, offset);
        }

        transform.set(matrix);
      }
    }
  }

  if (components.has("fill")) {
    io::json::JSON& fill = components["fill"];

    if (fill.type() == io::json::JSON::Class::String) {
      const std::string fill_op = fill.to_string();

      if (fill_op == "add") {
      } else if (fill_op == "remove") {
      }
    } else {
      if (fill.has("color")) {
        const vec4 color = fill["color"].to_vec4();
        MODIFY_SELECTED(FillComponent, color(color));
      }

      if (fill.has("rule")) {
        const renderer::FillRule rule = static_cast<renderer::FillRule>(fill["rule"].to_int());
        MODIFY_SELECTED(FillComponent, rule(rule));
      }

      if (fill.has("visible")) {
        const bool visible = fill["visible"].to_bool();
        MODIFY_SELECTED(FillComponent, visible(visible));
      }
    }
  }

  if (components.has("stroke")) {
    io::json::JSON& stroke = components["stroke"];

    if (stroke.type() == io::json::JSON::Class::String) {
      const std::string stroke_op = stroke.to_string();

      if (stroke_op == "add") {
      } else if (stroke_op == "remove") {
      }
    } else {
      if (stroke.has("color")) {
        const vec4 color = stroke["color"].to_vec4();
        MODIFY_SELECTED(StrokeComponent, color(color));
      }

      if (stroke.has("width")) {
        const float width = stroke["width"].to_float();
        MODIFY_SELECTED(StrokeComponent, width(width));
      }

      if (stroke.has("cap")) {
        const renderer::LineCap cap = static_cast<renderer::LineCap>(stroke["cap"].to_int());
        MODIFY_SELECTED(StrokeComponent, cap(cap));
      }

      if (stroke.has("join")) {
        const renderer::LineJoin join = static_cast<renderer::LineJoin>(stroke["join"].to_int());
        MODIFY_SELECTED(StrokeComponent, join(join));
      }

      if (stroke.has("miter_limit")) {
        const float miter_limit = stroke["miter_limit"].to_float();
        MODIFY_SELECTED(StrokeComponent, miter_limit(miter_limit));
      }

      if (stroke.has("visible")) {
        const bool visible = stroke["visible"].to_bool();
        MODIFY_SELECTED(StrokeComponent, visible(visible));
      }
    }
  }

  request_render({false, false});
}

bool Editor::render_frame(const double time)
{
  if (!m_render_request.has_value())
    return false;

  if (m_render_request->frame_rate < 60 &&
      time - m_last_render_time < 1000.0 / m_render_request->frame_rate)
  {
    return false;
  }

  scene().render(m_render_request->ignore_cache);

  if (m_render_request->update_ui) {
#ifdef EMSCRIPTEN
    msgbus_send(0);
#endif
  }

  m_render_request.reset();

  return true;
}

}  // namespace graphick::editor
