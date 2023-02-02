#include "renderer.h"

#include "../common.h"

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#else
#include <glad/glad.h>
#endif

Renderer* Renderer::s_instance = nullptr;

static const size_t max_vertex_buffer_size = 2 * 100000;
static const size_t max_vertex_count = max_vertex_buffer_size / sizeof(Vertex);
static const size_t max_index_count = max_vertex_count * 3;

void Renderer::init() {
  assert(!s_instance);
  s_instance = new Renderer();

#ifdef EMSCRIPTEN
  EmscriptenWebGLContextAttributes attr;

  // attr.alpha = false;
  attr.antialias = true;
  attr.premultipliedAlpha = true;

  emscripten_webgl_init_context_attributes(&attr);

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);
#endif

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  get()->m_shaders.create_shaders();

  get()->m_data.vertex_buffer = new Vertex[max_vertex_count];
  get()->m_data.index_buffer = new uint32_t[max_index_count];

  glGenBuffers(1, &get()->m_data.vertex_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, get()->m_data.vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, max_vertex_count * sizeof(Vertex), nullptr, GL_STREAM_DRAW);

  get()->m_shaders.use("pen");
  get()->m_shaders.set_attribute("aPosition", 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
  get()->m_shaders.set_attribute("aColor", 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));

  glGenBuffers(1, &get()->m_data.index_buffer_object);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, get()->m_data.index_buffer_object);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_index_count * sizeof(uint32_t), nullptr, GL_STREAM_DRAW);
}

void Renderer::shutdown() {
  delete[] get()->m_data.vertex_buffer;
  delete[] get()->m_data.index_buffer;

  delete s_instance;
}

void Renderer::resize(const vec2& size) {
  get()->m_size = size;

  glViewport(0, 0, (GLsizei)size.x, (GLsizei)size.y);
}

void Renderer::begin_frame(const vec2& position, float zoom) {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  get()->set_viewport(position, zoom);
  get()->begin_batch();
}

void Renderer::end_frame() {
  s_instance->end_batch();
  s_instance->flush();
}


void Renderer::draw(const Geometry& geometry) {
  get()->add_to_batch(geometry);
}

void Renderer::set_viewport(const vec2& position, float zoom) {
  mat3 scaling = { 2.0f * zoom / m_size.x, 0.0f, 0.0f, 0.0f, -2.0f * zoom / m_size.y, 0.0f, 0.0f, 0.0f, 1.0f };
  mat3 translation = { 1.0f, 0.0f, (-m_size.x / zoom + 2 * position[0]) / 2.0f, 0.0f, 1.0f, (-m_size.y / zoom + 2 * position[1]) / 2.0f, 0.0f, 0.0f, 1.0f };

  mat3 view_projection_matrix = scaling * translation;

  m_shaders.set_global_uniform("uViewProjectionMatrix", view_projection_matrix);
}

void Renderer::begin_batch() {
  m_data.vertex_buffer_ptr = m_data.vertex_buffer;
  m_data.index_buffer_ptr = m_data.index_buffer;
}

void Renderer::add_to_batch(const Geometry& geometry) {
  if (m_data.index_count + geometry.indices.size() >= max_index_count ||
    m_data.vertex_count + geometry.vertices.size() >= max_vertex_count) {
    end_batch();
    flush();
    begin_batch();
  }

  for (size_t i = 0; i < geometry.vertices.size(); i++) {
    const Vertex& vertex = geometry.vertices[i];
    m_data.vertex_buffer_ptr->position = vertex.position;
    m_data.vertex_buffer_ptr->color = vertex.color;
    m_data.vertex_buffer_ptr++;
  }

  for (size_t i = 0; i < geometry.indices.size(); i++) {
    *(m_data.index_buffer_ptr) = m_data.vertex_count + geometry.indices[i];
    m_data.index_buffer_ptr++;
  }

  m_data.vertex_count += (uint32_t)geometry.vertices.size();
  m_data.index_count += (uint32_t)geometry.indices.size();
}

void Renderer::end_batch() {
  GLsizeiptr vertex_buffer_size = (uint8_t*)m_data.vertex_buffer_ptr - (uint8_t*)m_data.vertex_buffer;
  GLsizeiptr index_buffer_size = (uint8_t*)m_data.index_buffer_ptr - (uint8_t*)m_data.index_buffer;

  glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, m_data.vertex_buffer);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size, m_data.index_buffer);
}

void Renderer::flush() {
  glDrawElements(GL_TRIANGLES, m_data.index_count, GL_UNSIGNED_INT, nullptr);

  m_data.vertex_count = 0;
  m_data.index_count = 0;
}
