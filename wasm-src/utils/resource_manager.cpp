#include "resource_manager.h"

#include "console.h"

#define SHADERS_LENGTH 7

static constexpr const char* shader_names[SHADERS_LENGTH] = {
  "default", "opaque_tile", "masked_tile", "line", "square", "circle", "gpu_path"
};

namespace Graphick::Utils {

  ResourceManager* ResourceManager::s_instance = nullptr;

  void ResourceManager::init() {
    if (s_instance != nullptr) {
      console::error("ResourceManager already initialized, call shutdown() before reinitializing!");
      return;
    }

    s_instance = new ResourceManager();

    s_instance->prefetch_shaders();
  }

  void ResourceManager::shutdown() {}

  std::string ResourceManager::get_shader(const std::string& name) {
    return s_instance->m_shaders.at(name);
  }

  void ResourceManager::prefetch_shaders() {
    static const char* shader_sources[SHADERS_LENGTH * 2] = {
      #include "../renderer/gpu/shaders/default.vs.glsl"
      ,
      #include "../renderer/gpu/shaders/default.fs.glsl"
      ,
      #include "../renderer/gpu/shaders/opaque_tile.vs.glsl"
      ,
      #include "../renderer/gpu/shaders/opaque_tile.fs.glsl"
      ,
      #include "../renderer/gpu/shaders/masked_tile.vs.glsl"
      ,
      #include "../renderer/gpu/shaders/masked_tile.fs.glsl"
      ,
      #include "../renderer/gpu/shaders/line.vs.glsl"
      ,
      #include "../renderer/gpu/shaders/line.fs.glsl"
      ,
      #include "../renderer/gpu/shaders/square.vs.glsl"
      ,
      #include "../renderer/gpu/shaders/square.fs.glsl"
      ,
      #include "../renderer/gpu/shaders/circle.vs.glsl"
      ,
      #include "../renderer/gpu/shaders/circle.fs.glsl"
      ,
      #include "../renderer/gpu/shaders/gpu_path.vs.glsl"
      ,
      #include "../renderer/gpu/shaders/gpu_path.fs.glsl"
    };

    for (unsigned int i = 0; i < SHADERS_LENGTH; ++i) {
      std::string name{ shader_names[i] };
      m_shaders.insert({ name + ".vs", shader_sources[i * 2 + 0] });
      m_shaders.insert({ name + ".fs", shader_sources[i * 2 + 1] });
    }
  }

}
