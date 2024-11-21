/**
 * @file resource_manager.cpp
 * @brief The file contains the implementation of the resource manager.
 */

#include "resource_manager.h"

#include "console.h"

#include "../lib/stb/stb_image.h"

#define SHADERS_LENGTH 6

static constexpr const char* shader_names[SHADERS_LENGTH] = {
    "tile", "fill", "line", "rect", "circle", "image"};

namespace graphick::utils {

ResourceManager* ResourceManager::s_instance = nullptr;

void ResourceManager::init()
{
  if (s_instance != nullptr) {
    console::error("ResourceManager already initialized, call shutdown() before reinitializing!");
    return;
  }

  s_instance = new ResourceManager();

  s_instance->prefetch_shaders();
}

void ResourceManager::shutdown() {}

std::string ResourceManager::get_shader(const std::string& name)
{
  return s_instance->m_shaders.at(name);
}

uuid ResourceManager::load_image(std::unique_ptr<uint8_t[]>&& data,
                                 const ivec2 size,
                                 const uint8_t channels)
{
  const uuid id = uuid();
  s_instance->m_images.insert(std::make_pair(id, ImageData{std::move(data), size, channels}));
  return id;
}

uuid ResourceManager::load_image(const uint8_t* data, const size_t size)
{
  int width, height, channels;

  // TODO: custom allocator for stb_image to avoid double copy
  uint8_t* buffer = stbi_load_from_memory(data, size, &width, &height, &channels, 0);

  const size_t buffer_size = width * height * channels;

  std::unique_ptr<uint8_t[]> image_data = std::make_unique<uint8_t[]>(buffer_size);

  memcpy(image_data.get(), buffer, buffer_size);
  stbi_image_free(buffer);

  return load_image(std::move(image_data), {width, height}, channels);
}

Image ResourceManager::get_image(const uuid id)
{
  const auto it = s_instance->m_images.find(id);
  if (it == s_instance->m_images.end()) {
    console::error("Image not found in cache!");
    return Image{nullptr, ivec2{0, 0}, 0};
  }

  return Image{it->second.data.get(), it->second.size, it->second.channels};
}

void ResourceManager::prefetch_shaders()
{
  static const char* shader_sources[SHADERS_LENGTH * 2] = {
#include "../renderer/gpu/shaders/tile.vs.glsl"
      ,
#include "../renderer/gpu/shaders/tile.fs.glsl"
      ,
#include "../renderer/gpu/shaders/fill.vs.glsl"
      ,
#include "../renderer/gpu/shaders/fill.fs.glsl"
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

}  // namespace graphick::utils
