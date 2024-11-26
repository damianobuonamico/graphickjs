/**
 * @file io/resource_manager.cpp
 * @brief The file contains the implementation of the resource manager.
 */

#include "resource_manager.h"

#include "../utils/console.h"

#include "../lib/stb/stb_image.h"
#include "../lib/stb/stb_truetype.h"

#define SHADERS_LENGTH 6

static constexpr const char* shader_names[SHADERS_LENGTH] = {
    "tile", "fill", "line", "rect", "circle", "image"};

namespace graphick::io {

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

uuid ResourceManager::load_image(const uint8_t* data, const size_t size)
{
  const uuid id = uuid();

  int width, height, channels;

  if (stbi_is_16_bit_from_memory(data, size)) {
    console::error("16bit images are not supported yet!");
    return uuid::null;
  }

  uint8_t* buffer = stbi_load_from_memory(data, size, &width, &height, &channels, 0);

  if (buffer == nullptr) {
    console::error("Failed to load image from memory!");
    return uuid::null;
  }

  s_instance->m_images.insert(
      std::make_pair(id, ImageData{buffer, ivec2(width, height), static_cast<uint8_t>(channels)}));

  return id;
}

uuid ResourceManager::load_font(const uint8_t* data, const size_t size)
{
  text::Font font(data, size);

  // TODO: load multiple fonts from the same file if needed
  if (!font.valid()) {
    console::error("Failed to load font from memory!");
    return uuid::null;
  }

  return s_instance->m_fonts.insert(std::make_pair(uuid(), std::move(font))).first->first;
}

Image ResourceManager::get_image(const uuid id)
{
  const auto it = s_instance->m_images.find(id);
  if (it == s_instance->m_images.end()) {
    console::error("Image not found in cache!");
    return Image{nullptr, ivec2{0, 0}, 0};
  }

  return Image{it->second.data, it->second.size, it->second.channels};
}

const text::Font& ResourceManager::get_font(const uuid id)
{
  const auto it = s_instance->m_fonts.find(id);

  if (it == s_instance->m_fonts.end()) {
    console::error("Font not found in cache!");
    // TODO: return a default font
    return s_instance->m_fonts.at(uuid::null);
  }

  return it->second;
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

ResourceManager::ImageData::ImageData(uint8_t* data, ivec2 size, uint8_t channels)
    : data(data), size(size), channels(channels)
{
}

ResourceManager::ImageData::ImageData(ImageData&& other) noexcept
    : data(other.data), size(other.size), channels(other.channels)
{
  other.data = nullptr;
}

ResourceManager::ImageData& ResourceManager::ImageData::operator=(ImageData&& other) noexcept
{
  if (this != &other) {
    data = other.data;
    size = other.size;
    channels = other.channels;

    other.data = nullptr;
  }

  return *this;
}

ResourceManager::ImageData::~ImageData()
{
  stbi_image_free(data);
}

}  // namespace graphick::io
