/**
 * @file resource_manager.h
 * @brief The file contains the definition of the resource manager.
 */

#pragma once

#include <string>
#include <unordered_map>

namespace graphick::utils {

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
  ResourceManager(const ResourceManager &) = delete;
  ResourceManager(ResourceManager &&) = delete;
  ResourceManager &operator=(const ResourceManager &) = delete;
  ResourceManager &operator=(ResourceManager &&) = delete;

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
  static std::string get_shader(const std::string &name);

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
 private:
  static ResourceManager *s_instance;  // The instance of the resource manager singleton.
};

}  // namespace graphick::utils
