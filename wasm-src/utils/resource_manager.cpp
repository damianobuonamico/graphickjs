/**
 * @file resource_manager.cpp
 * @brief The file contains the implementation of the resource manager.
 */

#include "resource_manager.h"

#include "console.h"

#define SHADERS_LENGTH 8

static constexpr const char* shader_names[SHADERS_LENGTH] =
  {"tile", "path", "boundary_span", "filled_span", "line", "rect", "circle", "image"};

namespace graphick::utils {

ResourceManager* ResourceManager::s_instance = nullptr;

void ResourceManager::init() {
  if (s_instance != nullptr) {
    console::error("ResourceManager already initialized, call shutdown() before reinitializing!");
    return;
  }

  s_instance = new ResourceManager();

  s_instance->prefetch_shaders();
}

void ResourceManager::shutdown() { }

std::string ResourceManager::get_shader(const std::string& name) { return s_instance->m_shaders.at(name); }

void ResourceManager::prefetch_shaders() {
  static const char* shader_sources[SHADERS_LENGTH * 2] = {
#include "../renderer/gpu/shaders/tile.vs.glsl"
    ,
#include "../renderer/gpu/shaders/tile.fs.glsl"
    ,
#include "../renderer/gpu/shaders/path.vs.glsl"
    ,
#include "../renderer/gpu/shaders/path.fs.glsl"
    ,
#include "../renderer/gpu/shaders/boundary_span.vs.glsl"
    ,
#include "../renderer/gpu/shaders/boundary_span.fs.glsl"
    ,
#include "../renderer/gpu/shaders/filled_span.vs.glsl"
    ,
#include "../renderer/gpu/shaders/filled_span.fs.glsl"
    ,
#include "../renderer/gpu/shaders/line.vs.glsl"
    ,
#include "../renderer/gpu/shaders/line.fs.glsl"
    ,
#include "../renderer/gpu/shaders/rect.vs.glsl"
    ,
#include "../renderer/gpu/shaders/rect.fs.glsl"
    ,
#include "../renderer/gpu/shaders/circle.vs.glsl"
    ,
#include "../renderer/gpu/shaders/circle.fs.glsl"
    ,
#include "../renderer/gpu/shaders/image.vs.glsl"
    ,
#include "../renderer/gpu/shaders/image.fs.glsl"
  };

  for (unsigned int i = 0; i < SHADERS_LENGTH; ++i) {
    std::string name{shader_names[i]};
    m_shaders.insert({name + ".vs", shader_sources[i * 2 + 0]});
    m_shaders.insert({name + ".fs", shader_sources[i * 2 + 1]});
  }
}

}
