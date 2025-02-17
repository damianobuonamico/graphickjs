/**
 * @file io/resource_manager.h
 * @brief The file contains the definition of the resource manager.
 *
 * @todo The resource manager should also handle generic file loading, fonts, etc.
 */

#pragma once

#include "image/image.h"
#include "text/font.h"

#include "../utils/uuid.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace graphick::io {

/**
 * @brief The class that represents the resource manager.
 *
 * It is responsible for loading and caching static resources such as shaders and fonts.
 */
class ResourceManager {
 public:
  /**
   * @brief Deleted copy, move constructor and assignment operators.
   */
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager(ResourceManager&&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;
  ResourceManager& operator=(ResourceManager&&) = delete;

  /**
   * @brief Initializes the resource manager.
   */
  static void init();

  /**
   * @brief Shuts down the resource manager.
   */
  static void shutdown();

  /**
   * @brief Retrieves the instance of the resource manager.
   *
   * @return The instance of the resource manager.
   */
  static std::string get_shader(const std::string& name);

  /**
   * @brief Loads an image into the cache.
   *
   * @param data The image data.
   * @param size The size of the image data.
   * @return The UUID of the image.
   */
  static uuid load_image(const uint8_t* data, const size_t size);

  /**
   * @brief Loads a font into the cache.
   *
   * @param data The font data.
   * @param size The size of the font data.
   * @return The UUID of the font.
   */
  static uuid load_font(const uint8_t* data, const size_t size);

  /**
   * @brief Loads the default font into the cache.
   *
   * @param data The font data.
   * @param size The size of the font data.
   * @return The UUID of the font (uuid::null)
   */
  static uuid load_default_font(const uint8_t* data, const size_t size);

  /**
   * @brief Retrieves an image from the cache.
   *
   * @param id The UUID of the image.
   * @return A lightweight wrapper around the image data.
   */
  static Image get_image(uuid id);

  static const text::Font& get_font(const uuid id);

 private:
  /**
   * @brief The struct that represents the image data in the cache.
   */
  struct ImageData {
    uint8_t* data;     // A pointer to the image data (allocated by stb_image).
    ivec2 size;        // The size of the image.
    uint8_t channels;  // The number of channels of the image.

    ImageData(uint8_t* data, ivec2 size, uint8_t channels);
    ImageData(const ImageData&) = delete;
    ImageData(ImageData&& other) noexcept;
    ImageData& operator=(const ImageData&) = delete;
    ImageData& operator=(ImageData&& other) noexcept;

    /**
     * @brief Custom destructor to call stbi_image_free.
     */
    ~ImageData();
  };

 private:
  /**
   * @brief Default constructor and destructor.
   */
  ResourceManager() = default;
  ~ResourceManager() = default;

  /**
   * @brief Prefetches the shaders and loads them into the cache.
   *
   * The shaders are stored inline in the source code due to the limitations of WebAssembly.
   */
  void prefetch_shaders();

 private:
  std::unordered_map<std::string, std::string> m_shaders;  // The cache of shaders.
  std::unordered_map<uuid, ImageData> m_images;            // The cache of images.
  std::unordered_map<uuid, text::Font> m_fonts;            // The cache of fonts.
 private:
  static ResourceManager* s_instance;  // The instance of the resource manager singleton.
};

}  // namespace graphick::io
