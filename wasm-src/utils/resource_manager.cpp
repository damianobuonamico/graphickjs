#include "resource_manager.h"

#include "console.h"

#define SHADERS_LENGTH 2

static constexpr const char* shader_names[SHADERS_LENGTH] = {
  "fill", "line"
};

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
    #include "../renderer/gpu/shaders/fill.vs.glsl"
    ,
    #include "../renderer/gpu/shaders/fill.fs.glsl"
    ,
    #include "../renderer/gpu/shaders/line.vs.glsl"
    ,
    #include "../renderer/gpu/shaders/line.fs.glsl"
    // #include "../renderer/gpu/shaders/blit.vs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/blit.fs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/clear.vs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/clear.fs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/stencil.vs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/stencil.fs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/reproject.vs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/reproject.fs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/fill.vs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/fill.fs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/tile.vs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/tile.fs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/tile_clip_combine.vs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/tile_clip_combine.fs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/tile_clip_copy.vs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/tile_clip_copy.fs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/tile_copy.vs.glsl"
    //   ,
    // #include "../renderer/gpu/shaders/tile_copy.fs.glsl"
  };

  for (unsigned int i = 0; i < SHADERS_LENGTH; ++i) {
    std::string name{ shader_names[i] };
    m_shaders.insert({ name + ".vs", shader_sources[i * 2 + 0] });
    m_shaders.insert({ name + ".fs", shader_sources[i * 2 + 1] });
  }
}
