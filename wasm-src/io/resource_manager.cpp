/**
 * @file io/resource_manager.cpp
 * @brief The file contains the implementation of the resource manager.
 */

#include "resource_manager.h"

#include "text/default_font.h"

#include "../utils/console.h"
#include "../utils/defines.h"

#include "../lib/stb/stb_image.h"
#include "../lib/stb/stb_truetype.h"

static uint8_t default_image_data[4] = {255, 0, 255, 255};

static const std::string shader_include_names[] = {"quadratic", "cubic", "texture"};

static const std::string shader_names[] = {"tile",
                                           "fill",
                                           "primitive",
#ifdef GK_DEBUG
                                           "debug_rect"
#endif
};

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

uuid ResourceManager::load_default_font(const uint8_t* data, const size_t size)
{
  text::Font font(data, size);

  if (!font.valid()) {
    console::error("Failed to load font from memory!");
    return uuid::null;
  }

  s_instance->m_fonts.erase(uuid::null);

  return s_instance->m_fonts.insert(std::make_pair(uuid::null, std::move(font))).first->first;
}

Image ResourceManager::get_image(const uuid id)
{
  const auto it = s_instance->m_images.find(id);

  if (it == s_instance->m_images.end()) {
    console::error("Image not found in cache!");
    return Image{default_image_data, ivec2(1), uint8_t(4)};
  }

  return Image{it->second.data, it->second.size, it->second.channels};
}

const text::Font& ResourceManager::get_font(const uuid id)
{
  const auto it = s_instance->m_fonts.find(id);

  if (it == s_instance->m_fonts.end()) {
    console::error("Font not found in cache!");
    return s_instance->m_fonts.at(uuid::null);
  }

  return it->second;
}

ResourceManager::ResourceManager()
{
  text::Font font(default_font_data, std::size(default_font_data));
  m_fonts.insert(std::make_pair(uuid::null, std::move(font)));

  ImageData image(default_image_data, ivec2(1), uint8_t(4));
  m_images.insert(std::make_pair(uuid::null, std::move(image)));
}

void ResourceManager::prefetch_shaders()
{
  static std::string shader_include_sources[std::size(shader_include_names)] = {
#include "../renderer/gpu/shaders/include/quadratic.glsl"
      ,
#include "../renderer/gpu/shaders/include/cubic.glsl"
      ,
#include "../renderer/gpu/shaders/include/texture.glsl"
  };

  static std::string shader_sources[std::size(shader_names) * 2] = {
#include "../renderer/gpu/shaders/tile.vs.glsl"
      ,
#include "../renderer/gpu/shaders/tile.fs.glsl"
      ,
#include "../renderer/gpu/shaders/fill.vs.glsl"
      ,
#include "../renderer/gpu/shaders/fill.fs.glsl"
      ,
#include "../renderer/gpu/shaders/primitive.vs.glsl"
      ,
#include "../renderer/gpu/shaders/primitive.fs.glsl"
#ifdef GK_DEBUG
      ,
#  include "../renderer/gpu/shaders/debug_rect.vs.glsl"
      ,
#  include "../renderer/gpu/shaders/debug_rect.fs.glsl"
#endif
  };

  for (unsigned int i = 0; i < std::size(shader_names); i++) {
    for (unsigned int j = 0; j < std::size(shader_include_names); j++) {
      std::string include = "#include \"" + shader_include_names[j] + ".glsl\"";

      for (int k = 0; k < 2; k++) {
        size_t pos = 0;

        while ((pos = shader_sources[i * 2 + k].find(include, pos)) != std::string::npos) {
          shader_sources[i * 2 + k].replace(pos, include.length(), shader_include_sources[j]);
          pos += shader_include_sources[j].length();
        }
      }
    }

    m_shaders.insert({shader_names[i] + ".vs", shader_sources[i * 2 + 0]});
    m_shaders.insert({shader_names[i] + ".fs", shader_sources[i * 2 + 1]});
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
