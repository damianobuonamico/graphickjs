#pragma once

#include <string>
#include <unordered_map>

class ResourceManager {
public:
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager(ResourceManager&&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;
  ResourceManager& operator=(ResourceManager&&) = delete;

  static void init();
  static void shutdown();

  static std::string get_shader(const std::string& name);
private:
  ResourceManager() = default;
  ~ResourceManager() = default;

  void prefetch_shaders();
private:
  std::unordered_map<std::string, std::string> m_shaders;
private:
  static ResourceManager* s_instance;
};
