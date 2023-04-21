#include "renderer.h"

#include "../common.h"

#ifdef __EMSCRIPTEN__
#ifndef EMSCRIPTEN
#define EMSCRIPTEN
#endif
#endif

#ifdef EMSCRIPTEN
#include <GLES3/gl32.h>
#include <emscripten/html5.h>
#else
#include <glad/glad.h>
#endif

#include "../math/vec3.h"
#include "../math/mat4.h"

#include <stdexcept>

Renderer* Renderer::s_instance = nullptr;

static int rendered = 0;

void Renderer::init() {
  assert(!s_instance);
  s_instance = new Renderer();

  const RendererSettings& settings = get()->m_settings;

#ifdef EMSCRIPTEN
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);

  attr.alpha = true;
  attr.premultipliedAlpha = true;
  attr.majorVersion = 2;
  attr.antialias = true;
  // attr.antialias = settings.antialiasing == Antialiasing::Hardware;
  attr.stencil = true;


  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);
#else
  if (settings.antialiasing == Antialiasing::Hardware) {
    glEnable(GL_MULTISAMPLE);
  }
#endif

  glLineWidth(1.0f);
  // TODO: Enable depth
  glDisable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_STENCIL_TEST);
  // glStencilFunc(GL_EQUAL, 1, 0xFF);
  // glStencilMask(0xFF);
  // glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
  // glStencilMask(0xFF);
  // glStencilFunc(GL_EQUAL, 1, 0xFF);
  // glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  get()->m_shaders.create_shaders();
  if (settings.antialiasing == Antialiasing::FXAA || settings.antialiasing == Antialiasing::MSAA) {
    get()->m_frame_buffer.init(settings.antialiasing == Antialiasing::MSAA, settings.msaa_samples);
  }

  get()->init_batch_renderer();
  get()->init_instance_renderer();
}

void Renderer::shutdown() {
  delete[] get()->m_data.vertex_buffer;
  delete[] get()->m_data.index_buffer;

  delete s_instance;
}

void Renderer::resize(const vec2& size, float dpr) {
  get()->m_size = size;
  get()->m_dpr = dpr;
  get()->m_frame_buffer.resize(size * dpr);
  glViewport(0, 0, (GLsizei)(size.x * dpr), (GLsizei)(size.y * dpr));
}

struct Vertex3D {
  vec3 position;
  float uniform_index = 2.0f;
};

struct Geometry3D {
  Geometry3D() = default;

  void push_quad(const vec3 position, float radius) {
    m_vertices.insert(m_vertices.end(), {
      { position + vec3{ -radius, -radius, 0 } },
      { position + vec3{ -radius,  radius, 0 } },
      { position + vec3{  radius,  radius, 0 } },
      { position + vec3{  radius, -radius, 0 } }
      });
    uint32_t offset = m_vertices.size();
    m_indices.insert(m_indices.end(), { offset - 4, offset - 3, offset - 2, offset - 4, offset - 2, offset - 1 });
  }

  std::vector<Vertex3D> m_vertices;
  std::vector<uint32_t> m_indices;
};

void Renderer::begin_frame(const vec2& position, float zoom) {
  get()->set_viewport(position, zoom);

  glClearColor(0.09f, 0.11f, 0.13f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_STENCIL_TEST);

  // Disable rendering to the color buffer
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  // Start using the stencil
  glEnable(GL_STENCIL_TEST);

  // Place a 1 where rendered
  glStencilFunc(GL_ALWAYS, 1, 1);

  // Replace where rendered
  glStencilOp(GL_REPLACE, GL_REPLACE, GL_INVERT);

  // glClearStencil(0);
  // glClear(GL_STENCIL_BUFFER_BIT);
  // glStencilFunc(GL_ALWAYS, 1, 0xFF);
  // glStencilMask(0xFF);
  // glStencilOp(GL_REPLACE, GL_REPLACE, GL_INVERT);

  // glClearColor(0.09f, 0.11f, 0.13f, 1.0f);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  // glStencilMask(0xFF);
  // glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

  Geometry3D geo;
  geo.push_quad({ 150.0f, 150.0f, 1.0f }, 20.0f);
  geo.push_quad({ 160.0f, 160.0f, 1.0f }, 20.0f);
  geo.push_quad({ 300.0f, 300.0f, 1.0f }, 20.0f);
  // geo.push_quad({ 100.0f, 100.0f, 3.5f }, 50.0f);
  // geo.push_quad({ 50.0f, 50.0f, 3.0f }, 50.0f);

  GLuint vao, vbo, ibo;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, geo.m_vertices.size() * sizeof(Vertex3D), geo.m_vertices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, geo.m_indices.size() * sizeof(uint32_t), geo.m_indices.data(), GL_STATIC_DRAW);

  get()->m_shaders.use("depth");
  get()->m_shaders.set_attribute("aPosition", 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (const void*)offsetof(Vertex3D, position));
  get()->m_shaders.set_attribute("aIndex", 1, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (const void*)offsetof(Vertex3D, uniform_index));

  std::vector<float> colors;

  colors.insert(colors.end(), { 0.8f, 0.2f, 0.2f, 1.0f });
  colors.insert(colors.end(), { 0.2f, 0.8f, 0.2f, 1.0f });
  colors.insert(colors.end(), { 0.2f, 0.2f, 0.8f, 1.0f });
  colors.insert(colors.end(), { 1.0f, 1.0f, 1.0f, 1.0f });

  get()->m_shaders.set_uniform("uZFar", 4.0f);
  get()->m_shaders.set_uniform("uColors", colors.data(), (int)(colors.size() / 4));

  glDrawElements(GL_TRIANGLES, geo.m_indices.size(), GL_UNSIGNED_INT, 0);

  // Reenable color
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  // Where a 1 was not rendered
  glStencilFunc(GL_NOTEQUAL, 1, 1);

  // Keep the pixel
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

  // glStencilFunc(GL_EQUAL, 1, 0xFF);
  // glClearColor(0.09f, 0.11f, 0.13f, 1.0f);
  // glClear(GL_COLOR_BUFFER_BIT);

  Geometry3D geo2;
  geo2.push_quad({ 150.0f, 150.0f, 0.5f }, 150.0f);

  glBufferData(GL_ARRAY_BUFFER, geo2.m_vertices.size() * sizeof(Vertex3D), geo2.m_vertices.data(), GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, geo.m_indices.size() * sizeof(uint32_t), geo2.m_indices.data(), GL_STATIC_DRAW);

  glDrawElements(GL_TRIANGLES, geo2.m_indices.size(), GL_UNSIGNED_INT, 0);

  glDeleteBuffers(1, &ibo);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);

  //Finished using stencil
  glDisable(GL_STENCIL_TEST);

  return;

  rendered = 0;
  get()->set_viewport(position, zoom);
  get()->m_frame_buffer.bind();

  glClearColor(0.09f, 0.11f, 0.13f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glStencilFunc(GL_ALWAYS, 1, 0xFF);
  glStencilMask(0xFF);

  get()->begin_batch();
}

void Renderer::end_frame() {
  return;

  // console::log("Entities Rendered", rendered);
  if (get()->m_last_call == RenderCall::Batch) {
    get()->end_batch();
    get()->flush();
  }

  glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  glStencilMask(0x00);
  glDisable(GL_DEPTH_TEST);

  get()->m_last_call = RenderCall::None;
  get()->m_frame_buffer.render();
}

void Renderer::push_overlay_layer(const vec2& position) {
  return;

  if (get()->m_last_call == RenderCall::Batch) {
    get()->end_batch();
    get()->flush();
  }

  get()->m_last_call = RenderCall::None;

  get()->set_viewport(position, 1.0f);
  get()->begin_batch();
}

void Renderer::draw(const Geometry& geometry) {
  return;

  rendered++;
  if (get()->m_last_call != RenderCall::Batch) {
    get()->bind_batch_renderer();
    get()->m_last_call = RenderCall::Batch;
  }

  get()->add_to_batch(geometry);
};

void Renderer::draw(const InstancedGeometry& geometry) {
  return;

  if (geometry.instances() == 0) {
    return;
  }

  get()->bind_instance_renderer();
  get()->m_last_call = RenderCall::Instance;

  get()->draw_instanced(geometry);
}

void Renderer::draw(const Texture& texture) {
  return;

  GLuint position_buffer;
  GLuint texture_buffer;
  GLuint index_buffer;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  float positions[] = {
    -30.0f, -30.0f, 0.0f, 0.0f, 30.0f, -30.0f, 1.0f, 0.0f, 30.0f, 30.0f, 1.0f, 1.0f, -30.0f, 30.0f, 0.0f, 1.0f,
  };

  glGenBuffers(1, &position_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

  // float coordinates[] = {
  //   0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
  // };

  // glGenBuffers(1, &texture_buffer);
  // glBindBuffer(GL_ARRAY_BUFFER, texture_buffer);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(coordinates), coordinates, GL_STATIC_DRAW);

  uint32_t indices[] = {
    0, 1, 2, 0, 2, 3
  };

  glGenBuffers(1, &index_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
  get()->m_shaders.use("image");
  get()->m_shaders.set_attribute("aVertexPosition", 4, GL_FLOAT, GL_FALSE, 0, 0);
  // glBindBuffer(GL_ARRAY_BUFFER, texture_buffer);
  // get()->m_shaders.set_attribute("aTextureCoord", 2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);


  texture.bind();

  get()->m_shaders.set_uniform("uSampler", 0);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

  get()->m_last_call = RenderCall::Image;
}

void Renderer::push_frame_buffer(vec2 size) {
  return;

  if (get()->m_last_call == RenderCall::Batch) {
    get()->end_batch();
    get()->flush();
  }

  if (get()->m_current_frame_buffer != nullptr) {
    get()->m_current_frame_buffer->resize(get()->m_size * get()->m_dpr);
  } else {
    get()->m_current_frame_buffer = new FrameBuffer(get()->m_shaders, get()->m_size * get()->m_dpr);
    get()->m_current_frame_buffer->init(get()->m_settings.antialiasing == Antialiasing::MSAA, get()->m_settings.msaa_samples);
  }

  get()->m_last_call = RenderCall::None;
  get()->m_current_frame_buffer->bind();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::pop_frame_buffer() {
  return;

  if (get()->m_last_call == RenderCall::Batch) {
    get()->end_batch();
    get()->flush();
  }

  get()->m_current_frame_buffer->render();
  get()->m_last_call = RenderCall::None;
  // delete get()->m_current_frame_buffer;
}

Renderer::Renderer(): m_frame_buffer(m_shaders, vec2{ 0.0f }) {}

void Renderer::init_batch_renderer() {
  m_data.vertex_buffer = new Vertex[m_data.max_vertex_count];
  m_data.index_buffer = new uint32_t[m_data.max_index_count];

  glGenVertexArrays(1, &m_data.vertex_array_object);
  glBindVertexArray(m_data.vertex_array_object);

  glGenBuffers(1, &m_data.vertex_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, m_data.vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, m_data.max_vertex_count * sizeof(Vertex), nullptr, GL_STREAM_DRAW);

  glGenBuffers(1, &m_data.index_buffer_object);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_data.index_buffer_object);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_data.max_index_count * sizeof(uint32_t), nullptr, GL_STREAM_DRAW);
}

void Renderer::init_instance_renderer(bool create_vertex_array) {
  if (create_vertex_array) glGenVertexArrays(1, &m_instanced_data.vertex_array_object);
  glBindVertexArray(m_instanced_data.vertex_array_object);

  glGenBuffers(1, &m_instanced_data.vertex_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, m_instanced_data.vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, m_instanced_data.max_vertex_count * sizeof(Vertex), nullptr, GL_STREAM_DRAW);

  glGenBuffers(1, &m_instanced_data.index_buffer_object);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_instanced_data.index_buffer_object);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_instanced_data.max_index_count * sizeof(uint32_t), nullptr, GL_STREAM_DRAW);

  glGenBuffers(1, &m_instanced_data.instance_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, m_instanced_data.instance_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, m_instanced_data.max_instance_count * sizeof(vec2), nullptr, GL_STREAM_DRAW);
}

void Renderer::bind_batch_renderer() {
  glBindVertexArray(m_data.vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, m_data.vertex_buffer_object);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_data.index_buffer_object);

  m_shaders.use("batched");
  m_shaders.set_attribute("aPosition", 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
  m_shaders.set_attribute("aColor", 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));
  m_shaders.set_attribute("aNormal", 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, normal));
  m_shaders.set_attribute("aMaxNormal", 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, max_normal));
}

void Renderer::bind_instance_renderer() {
  if (m_last_call == RenderCall::Batch) {
    end_batch();
    flush();
  }

  glBindVertexArray(m_instanced_data.vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, m_instanced_data.vertex_buffer_object);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_instanced_data.index_buffer_object);

  m_shaders.use("instanced");
  m_shaders.set_attribute("aPosition", 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
  m_shaders.set_attribute("aColor", 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));
  m_shaders.set_attribute("aNormal", 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, normal));
  m_shaders.set_attribute("aMaxNormal", 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, max_normal));

  glBindBuffer(GL_ARRAY_BUFFER, m_instanced_data.instance_buffer_object);
  m_shaders.set_attribute("aTranslation", 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (const void*)0);
  glVertexAttribDivisor(4, 1);

  glBindBuffer(GL_ARRAY_BUFFER, m_instanced_data.vertex_buffer_object);
}

void Renderer::set_viewport(const vec2& position, float zoom) {
  // mat3 scaling = { 2.0f * zoom / m_size.x, 0.0f, 0.0f, 0.0f, -2.0f * zoom / m_size.y, 0.0f, 0.0f, 0.0f, 1.0f };
  // mat3 translation = { 1.0f, 0.0f, (-m_size.x / zoom + 2 * position[0]) / 2.0f, 0.0f, 1.0f, (-m_size.y / zoom + 2 * position[1]) / 2.0f, 0.0f, 0.0f, 1.0f };

  // mat3 view_projection_matrix = scaling * translation;

  float half_width = -m_size.x / 2.0f / zoom;
  float half_height = m_size.y / 2.0f / zoom;

  float right = -half_width;
  float left = half_width;
  float top = -half_height;
  float bottom = half_height;

  mat4 proj = {
    2.0f / (right - left), 0.0f, 0.0f, 0.0f,
    0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
    0.0f, 0.0f, -1.0f, 0.0f,
    -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0f, 1.0f
  };

  mat4 translation = {
    1.0f, 0.0f, 0.0f, (-m_size.x / zoom + 2 * position[0]) / 2.0f,
    0.0f, 1.0f, 0.0f, (-m_size.y / zoom + 2 * position[1]) / 2.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  m_shaders.set_view_projection_matrix(proj * translation);
  m_shaders.set_zoom(zoom);
}

void Renderer::begin_batch() {
  m_data.vertex_buffer_ptr = m_data.vertex_buffer;
  m_data.index_buffer_ptr = m_data.index_buffer;
}

// TODO: handle large geometry
void Renderer::add_to_batch(const Geometry& geometry) {
  if (m_data.index_count + geometry.indices().size() >= m_data.max_index_count ||
    m_data.vertex_count + geometry.vertices().size() >= m_data.max_vertex_count ||
    m_data.primitive != geometry.primitive()) {
    end_batch();
    flush();
    begin_batch();

    if (geometry.indices().size() >= m_data.max_index_count || geometry.vertices().size() >= m_data.max_vertex_count) {
      console::log("Geometry is too large to fit in a single batch.");
      throw std::invalid_argument("Geometry is too large to fit in a single batch.");
      return;
    }
  }

  for (size_t i = 0; i < geometry.vertices().size(); i++) {
    const Vertex& vertex = geometry.vertices()[i];
    m_data.vertex_buffer_ptr->position = vertex.position;
    m_data.vertex_buffer_ptr->color = vertex.color;
    m_data.vertex_buffer_ptr->normal = vertex.normal;
    m_data.vertex_buffer_ptr->max_normal = vertex.max_normal;
    m_data.vertex_buffer_ptr++;
  }

  for (size_t i = 0; i < geometry.indices().size(); i++) {
    *(m_data.index_buffer_ptr) = m_data.vertex_count + geometry.indices()[i];
    m_data.index_buffer_ptr++;
  }

  m_data.vertex_count += (uint32_t)geometry.vertices().size();
  m_data.index_count += (uint32_t)geometry.indices().size();
  m_data.primitive = geometry.primitive();
}

void Renderer::end_batch() {
  GLsizeiptr vertex_buffer_size = (uint8_t*)m_data.vertex_buffer_ptr - (uint8_t*)m_data.vertex_buffer;
  GLsizeiptr index_buffer_size = (uint8_t*)m_data.index_buffer_ptr - (uint8_t*)m_data.index_buffer;

  if (vertex_buffer_size == 0 || index_buffer_size == 0) {
    return;
  }

  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size, m_data.index_buffer);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, m_data.vertex_buffer);
}

void Renderer::flush() {
  if (m_data.vertex_count == 0 || m_data.index_count == 0) {
    return;
  }

  glDrawElements(m_data.primitive, m_data.index_count, GL_UNSIGNED_INT, nullptr);

  m_data.vertex_count = 0;
  m_data.index_count = 0;

  m_data.vertex_buffer_ptr = m_data.vertex_buffer;
  m_data.index_buffer_ptr = m_data.index_buffer;
}

void Renderer::draw_instanced(const InstancedGeometry& geometry) {
  m_instanced_data.instances = geometry.instances();

  if (
    geometry.vertices().size() > m_instanced_data.max_vertex_count / 2 ||
    geometry.indices().size() > m_instanced_data.max_index_count / 2 ||
    geometry.instances() > m_instanced_data.max_instance_count / 2
    ) {
    if (
      geometry.vertices().size() > m_instanced_data.max_vertex_count ||
      geometry.indices().size() > m_instanced_data.max_index_count ||
      geometry.instances() > m_instanced_data.max_instance_count
      ) {
      glDeleteBuffers(1, &m_instanced_data.vertex_buffer_object);
      glDeleteBuffers(1, &m_instanced_data.index_buffer_object);
      glDeleteBuffers(1, &m_instanced_data.instance_buffer_object);

      m_instanced_data.max_vertex_count = std::max((uint32_t)geometry.vertices().size(), m_instanced_data.max_vertex_count);
      m_instanced_data.max_index_count = std::max((uint32_t)geometry.indices().size(), m_instanced_data.max_index_count);
      m_instanced_data.max_instance_count = std::max(geometry.instances(), m_instanced_data.max_instance_count);

      init_instance_renderer(false);
      bind_instance_renderer();
    }

    m_instanced_data.last_allocation_usage = 0;
  } {
    m_instanced_data.last_allocation_usage++;

    if (
      m_instanced_data.last_allocation_usage > 1000 && (
        m_instanced_data.max_vertex_count > 100 ||
        m_instanced_data.max_index_count > 200 ||
        m_instanced_data.max_instance_count > 100
        )) {
      glDeleteVertexArrays(1, &m_instanced_data.vertex_array_object);
      glDeleteBuffers(1, &m_instanced_data.vertex_buffer_object);
      glDeleteBuffers(1, &m_instanced_data.index_buffer_object);
      glDeleteBuffers(1, &m_instanced_data.instance_buffer_object);

      m_instanced_data.max_vertex_count = 100;
      m_instanced_data.max_index_count = m_instanced_data.max_vertex_count * 2;
      m_instanced_data.max_instance_count = 100;

      init_instance_renderer(false);
      bind_instance_renderer();

      m_instanced_data.last_allocation_usage = 0;
    }
  }

  glBufferSubData(GL_ARRAY_BUFFER, 0, geometry.vertices().size() * sizeof(Vertex), geometry.vertices().data());
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, geometry.indices().size() * sizeof(uint32_t), geometry.indices().data());

  glBindBuffer(GL_ARRAY_BUFFER, m_instanced_data.instance_buffer_object);
  glBufferSubData(GL_ARRAY_BUFFER, 0, geometry.translations().size() * sizeof(vec2), geometry.translations().data());

  glDrawElementsInstanced(geometry.primitive(), (GLsizei)geometry.indices().size(), GL_UNSIGNED_INT, nullptr, m_instanced_data.instances);
}
