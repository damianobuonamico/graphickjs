/**
 * @file renderer/gpu/opengl/gl_device.h
 * @brief The file contains the definition of the OpenGL GPU device.
 */

#pragma once

#include "../render_state.h"

#include <memory>
#include <string>
#include <vector>

namespace graphick::renderer::GPU::GL {

/**
 * @brief The OpenGL render state
 */
struct GLState : public RenderState {
  GLState() : RenderState() {}
};

/**
 * @brief The class that represents the OpenGL GPU device.
 *
 * The device is responsible for creating and managing GPU resources. It is also responsible for
 * executing GPU commands.
 */
class GLDevice {
 public:
  /**
   * @brief This class is a singleton, so every public constructor/destructor is deleted.
   */
  GLDevice(const GLDevice &) = delete;
  GLDevice(GLDevice &&) = delete;
  GLDevice &operator=(const GLDevice &) = delete;
  GLDevice &operator=(GLDevice &&) = delete;

  /**
   * @brief Initializes the device with the given version.
   *
   * Will throw if initialization and compile versions are not compatible.
   *
   * @param version Backend version to initialize the device with.
   */
  static void init(const DeviceVersion version);

  /**
   * @brief Shuts down the device.
   *
   * It is necessary to call this method before reinitializing the device.
   */
  static void shutdown();

  /**
   * @brief Returns the current backend name.
   *
   * @return The backend name.
   */
  inline static const std::string &backend_name()
  {
    return s_device->m_backend_name;
  }

  /**
   * @brief Returns the current device name.
   *
   * @return The device name.
   */
  inline static const std::string &device_name()
  {
    return s_device->m_device_name;
  }

  /**
   * @brief Gets the maximum number of vertex uniform vectors.
   *
   * @return The maximum number of vertex uniform vectors.
   */
  inline static size_t max_vertex_uniform_vectors()
  {
    return static_cast<size_t>(s_device->m_max_vertex_uniform_vectors);
  }

  /**
   * @brief Gets the maximum number of texture image units in the fragment shader.
   *
   * @return The maximum number of texture image units.
   */
  inline static size_t max_texture_image_units()
  {
    return static_cast<size_t>(s_device->m_max_texture_image_units);
  }

  /**
   * @brief Prepares the GPU to execute commands.
   */
  static void begin_commands();

  /**
   * @brief Flushes the GPU commands.
   *
   * @return The GPU time.
   */
  static size_t end_commands();

  /**
   * @brief Sets the viewport size.
   *
   * @param viewport The new viewport.
   */
  static void set_viewport(const irect viewport);

  /**
   * @brief Sets the color mask.
   *
   * @param red Whether to enable red color channel.
   * @param green Whether to enable green color channel.
   * @param blue Whether to enable blue color channel.
   * @param alpha Whether to enable alpha color channel.
   */
  static void set_color_mask(const bool red, const bool green, const bool blue, const bool alpha);

  /**
   * @brief Clears the current render target.
   *
   * @param ops The clear operations.
   */
  static void clear(const ClearOps &ops);

  /**
   * @brief Creates a new shader program.
   *
   * @param name The name of the program
   * @return The new program.
   */
  static GLProgram create_program(
      const std::string &name,
      const std::vector<std::pair<std::string, std::string>> &variables = {});

  /**
   * @brief Queries the location of the uniform with the given name in the given program.
   *
   * @param program The program to search the uniform location into.
   * @param name The uniform name to query.
   * @return The location of the given uniform.
   */
  static GLUniform get_uniform(const GLProgram &program, const std::string &name);

  /**
   * @brief Creates a new texture uniform.
   *
   * @param program The program to create the texture uniform for.
   * @param name The name of the texture uniform.
   * @return The new texture uniform.
   */
  static GLTextureUniform get_texture_uniform(GLProgram &program, const std::string &name);

  /**
   * @brief Creates a new array of textures uniform.
   *
   * @param program The program to create the textures uniform for.
   * @param name The name of the textures uniform.
   * @return The new textures uniform.
   */
  static GLTexturesUniform get_textures_uniform(GLProgram &program,
                                                const std::string &name,
                                                const size_t count);

  /**
   * @brief Queries the location of the attribute with the given name in the given program.
   *
   * @param program The program to search the attribute location into.
   * @param name The attribute name to query.
   * @return The location of the given attribute or std::nullopt if not found.
   */
  static GLVertexAttribute get_vertex_attribute(const GLProgram &program, const std::string &name);

  /**
   * @brief Draws the binded index array with the given index count.
   *
   * @param index_count The number of indices to draw.
   * @param render_state The render state to use.
   */
  static void draw_elements(const size_t index_count, const RenderState &render_state);

  /**
   * @brief Draws the binded vertex array.
   *
   * @param vertex_count The number of vertices to draw.
   * @param render_state The render state to use.
   */
  static void draw_arrays(const size_t vertex_count, const RenderState &render_state);

  /**
   * @brief Draws the binded vertex array with instancing.
   *
   * @param vertex_count The number of vertices to draw.
   * @param instance_count The number of instances to draw.
   * @param render_state The render state to use.
   */
  static void draw_arrays_instanced(const size_t vertex_count,
                                    const size_t instance_count,
                                    const RenderState &render_state);

  /**
   * @brief Blits the given framebuffer to the default framebuffer.
   *
   * @param src The source framebuffer.
   * @param src_rect The source rectangle.
   * @param dst_rect The destination rectangle.
   * @param reverse Whether to reverse the blit (default framebuffer to provided framebuffer).
   */
  static void blit_framebuffer(const GLFramebuffer &src,
                               const irect src_rect,
                               const irect dst_rect,
                               const bool reverse);

 private:
  /**
   * @brief GLDevice constructor.
   */
  GLDevice(const DeviceVersion version);

  /**
   * @brief GLDevice destructor.
   */
  ~GLDevice();

  /**
   * @brief Sets the given uniforms to the correct values.
   *
   * @param program The program to bind the uniforms to.
   * @param uniforms The uniforms to set.
   */
  void set_uniforms(const GLProgram &program, const std::vector<UniformBinding> &uniforms);

  /**
   * @brief Sets the given textures to the correct texture units.
   *
   * @param program The program to bind the textures to.
   * @param textures The textures to set.
   */
  void set_textures(const GLProgram &program, const std::vector<TextureBinding> &textures);

  /**
   * @brief Sets the given textures to the correct texture units.
   *
   * @param program The program to bind the textures to.
   * @param textures The texture arrays to set.
   */
  void set_texture_arrays(const GLProgram &program,
                          const std::vector<TextureArrayBinding> &textures);

  /**
   * @brief Updates the render state if necessary.
   *
   * @param render_state The render state to update.
   */
  void set_render_state(const RenderState &render_state);

 private:
  std::string m_backend_name;          // The backend name.
  std::string m_device_name;           // The device name.
  std::string m_glsl_version_spec;     // The GLSL version specification.

  GLuint m_timer_query;                // The timer query.
  GLint m_max_vertex_uniform_vectors;  // The maximum number of vertex uniform vectors.
  GLint m_max_texture_image_units;     // The maximum number of texture image units in the fragment
                                       // shader.

  GLState m_state;                     // The current state.
 private:
  static GLDevice *s_device;
};
}  // namespace graphick::renderer::GPU::GL
