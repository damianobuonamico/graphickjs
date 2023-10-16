#include "scene.h"

#include "entity.h"

#include "../input/input_manager.h"

#include "../../renderer/renderer.h"

#include "../../history/command_history.h"
#include "../../history/commands.h"

// TEMP
#include "../../math/math.h"

#include "../../utils/debugger.h"
#include "../../utils/console.h"
#include "../../utils/misc.h"

namespace Graphick::History {

  class InsertInRegistryCommand : public Command {
  public:
    InsertInRegistryCommand(Editor::Scene* scene, std::unordered_map<uuid, entt::entity>* entities, uuid id) :
      Command(Type::InsertInRegistry),
      m_scene(scene), m_entities(entities), m_ids({ id })
    {
      Editor::Input::PenTool* pen = scene->tool_state.pen();
      if (!pen) return;

      m_pen = pen->pen_element() == id;
    }

    ~InsertInRegistryCommand() override {
      if (m_should_destroy) {
        for (uuid id : m_ids) {
          auto it = m_entities->find(id);
          if (it == m_entities->end()) continue;

          entt::entity entity = it->second;

          m_scene->selection.deselect(id);
          m_scene->m_registry.destroy(entity);
          m_entities->erase(it);
        }

        console::log("Destroyed entities", m_ids.size());
      }

      Command::~Command();
    }

    inline virtual void execute() override {
      m_should_destroy = false;

      m_scene->selection.clear();
      for (uuid id : m_ids) {
        m_scene->selection.select(id);
      }

      if (!m_pen) return;
      
      Editor::Input::PenTool* pen = m_scene->tool_state.pen();
      if (!pen) return;

      pen->set_pen_element(m_ids.back());
    }

    inline virtual void undo() override {
      m_should_destroy = true;

      for (uuid id : m_ids) {
        m_scene->selection.deselect(id);
      }

      m_scene->tool_state.reset_tool();
    }

    virtual bool merge_with(std::unique_ptr<Command>& command) override {
      if (command->type != Type::InsertInRegistry) return false;

      InsertInRegistryCommand* casted_command = static_cast<InsertInRegistryCommand*>(command.get());

      if (&casted_command->m_scene->m_registry != &this->m_scene->m_registry || casted_command->m_entities != this->m_entities || casted_command->m_should_destroy != this->m_should_destroy) return false;
      casted_command->m_ids.insert(casted_command->m_ids.end(), m_ids.begin(), m_ids.end());

      return true;
    }

    inline virtual uintptr_t pointer() override {
      return reinterpret_cast<uintptr_t>(&m_scene->m_registry);
    }
  private:
    Editor::Scene* m_scene;
    std::unordered_map<uuid, entt::entity>* m_entities;
    std::vector<uuid> m_ids;

    bool m_should_destroy = false;
    bool m_pen = false;
  };

}

namespace Graphick::Editor {

  Scene::Scene() : selection(this) {}

  Scene::Scene(const Scene& other) : m_entities(other.m_entities), m_order(other.m_order), selection(this), viewport(other.viewport) {
    m_registry.assign(other.m_registry.data(), other.m_registry.data() + other.m_registry.size(), other.m_registry.released());
  }

  Scene::Scene(Scene&& other) noexcept :
    m_registry(std::move(other.m_registry)),
    m_entities(std::move(other.m_entities)),
    selection(this) {}

  Scene::~Scene() {}

  Entity Scene::create_entity(const std::string& tag) {
    return create_entity(uuid(), tag);
  }

  Entity Scene::create_entity(uuid id, const std::string& tag) {
    Entity entity = { m_registry.create(), this };

    entity.add_component<IDComponent>(id);
    entity.add_component<TagComponent>(tag.empty() ? "Entity" : tag);

    m_entities[id] = entity;
    m_order.push_back(entity);

    // TODO: reset pen tool state on undo
    History::CommandHistory::add(std::make_unique<History::InsertInRegistryCommand>(this, &m_entities, id));

    return entity;
  }

  void Scene::delete_entity(Entity entity) {
    // TODO: implement deletion and z-ordering
    // m_order.erase(std::remove(m_order.begin(), m_order.end(), entity), m_order.end());
    // m_entities.erase(entity.id());
    // m_registry.destroy(entity);
  }

  bool Scene::has_entity(const uuid id) const {
    return m_entities.find(id) != m_entities.end();
  }

  Entity Scene::get_entity(const uuid id) {
    return { m_entities.at(id), this };
  }

  uuid Scene::entity_at(const vec2 position, bool deep_search, float threshold) {
    for (auto it = m_order.rbegin(); it != m_order.rend(); it++) {
      if (m_registry.all_of<PathComponent, TransformComponent>(*it)) {
        const auto& path = m_registry.get<PathComponent>(*it).path;
        const vec2 translation = m_registry.get<TransformComponent>(*it).position.get();
        const uuid id = m_registry.get<IDComponent>(*it).id;
        // auto pos = transform.Map(position.x, position.y);
        // if (path.is_inside({ (float)pos.X, (float)pos.Y }, deep_search, threshold)) return id;
        if (path.is_inside(position - translation, deep_search && selection.has(id), threshold)) {
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

    auto view = get_all_entities_with<IDComponent, PathComponent, TransformComponent>();

    for (entt::entity entity : view) {
      OPTICK_EVENT("entity_in_rect");

      auto components = view.get<IDComponent, PathComponent, TransformComponent>(entity);

      const uuid id = std::get<0>(components).id;
      const Renderer::Geometry::Path& path = std::get<1>(components).path;
      const vec2 position = std::get<2>(components).position.get();

      if (deep_search) {
        vertices.clear();

        Math::rect selection_rect = rect - position;

        if (Math::is_rect_in_rect(path.bounding_rect(), selection_rect)) {
          entities.insert({ id, Selection::SelectionElementEntry{ { 0 } } });
          continue;
        }

        vertices.reserve(path.segments().size() + 1);

        if (path.intersects(selection_rect, vertices)) {
          entities.insert({ id, Selection::SelectionElementEntry{ vertices } });
        }
      } else {
        if (path.intersects(rect - position)) {
          entities.insert({ id, Selection::SelectionElementEntry{ { 0 } } });
        }
      }
    }

    return entities;
  }

  Entity Scene::create_element(const std::string& tag) {
    Entity entity = create_entity(tag);

    entity.add_component<CategoryComponent>(CategoryComponent::Selectable);
    entity.add_component<PathComponent>(entity.id());
    entity.add_component<TransformComponent>();

    return entity;
  }

  Entity Scene::create_element(Renderer::Geometry::Path& path, const std::string& tag) {
    Entity entity = create_entity(tag);

    entity.add_component<CategoryComponent>(CategoryComponent::Selectable);
    entity.add_component<PathComponent>(entity.id(), path);
    entity.add_component<TransformComponent>();

    return entity;
  }

  void Scene::render() const {
    GK_TOTAL("Scene::render");
    OPTICK_EVENT();

    const Math::rect visible_rect = viewport.visible();

    Renderer::Renderer::begin_frame({
      Math::round(IVEC2_TO_VEC2(viewport.size()) * viewport.dpr()),
      viewport.position(),
      viewport.zoom() * viewport.dpr(),
      vec4{1.0f, 1.0f, 1.0f, 1.0f}
      });

    const size_t z_far = m_order.size() + 1;
    float z_index = 1.0f;

    // TODO: maybe check for rehydration only if actions performed are destructive
    bool should_rehydrate = true;

    {
      OPTICK_EVENT("Render Entities");

      auto& selected = selection.selected();
      auto& temp_selected = selection.temp_selected();

      for (auto it = m_order.rbegin(); it != m_order.rend(); it++) {
        if (!m_registry.all_of<IDComponent, PathComponent, TransformComponent>(*it)) continue;

        auto components = m_registry.get<IDComponent, PathComponent, TransformComponent>(*it);

        const uuid id = std::get<0>(components).id;
        const Renderer::Geometry::Path& path = std::get<1>(components).path;
        const vec2 position = std::get<2>(components).position.get();

        if (!has_entity(id)) return;

        if (should_rehydrate) {
          path.rehydrate_cache();
        }

        if (!Math::does_rect_intersect_rect(path.large_bounding_rect() + position, visible_rect)) continue;

        if (m_registry.all_of<FillComponent>(*it)) {
          Renderer::Renderer::draw(path, (/*z_far - */z_index++) / z_far, position, m_registry.get<FillComponent>(*it).color);
        } else {
          Renderer::Renderer::draw(path, (/*z_far - */z_index++) / z_far, position);
        }

        if (selected.find(id) != selected.end() || temp_selected.find(id) != temp_selected.end()) {
          Renderer::Renderer::draw_outline(id, path, position);
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

      tool_state.active().render_overlays();
    }

    Renderer::Renderer::end_frame();

    GK_DEBUGGER_RENDER(IVEC2_TO_VEC2(viewport.size()) * viewport.dpr());
  }

}
