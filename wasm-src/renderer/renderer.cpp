#include "renderer.h"

#include "../common.h"
#include "vertex.h"

#include <emscripten/html5.h>
#include <GLES2/gl2.h>

static const size_t max_vertex_buffer_size = 2 * 100000;
static const size_t max_vertex_count = max_vertex_buffer_size / sizeof(Vertex);
static const size_t max_index_count = max_vertex_count * 3;

ShaderManager Renderer::s_shaders;
Renderer::RendererData Renderer::s_data;
vec2 Renderer::s_size;

void Renderer::resize(const int width, const int height) {
  s_size.x = width;
  s_size.y = height;

  glViewport(0, 0, width, height);
}

void Renderer::init() {
  EmscriptenWebGLContextAttributes attr;

  attr.alpha = true;
  attr.antialias = true;
  attr.premultipliedAlpha = true;

  emscripten_webgl_init_context_attributes(&attr);

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  s_shaders.create_shaders();

  s_data.vertex_buffer = new Vertex[max_vertex_count];
  s_data.index_buffer = new uint32_t[max_index_count];

  glGenBuffers(1, &s_data.vertex_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, s_data.vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, max_vertex_count * sizeof(Vertex), nullptr, GL_STREAM_DRAW);

  s_shaders.use("pen");
  s_shaders.set_attribute("aPosition", 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));

  glGenBuffers(1, &s_data.index_buffer_object);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_data.index_buffer_object);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_index_count * sizeof(uint32_t), nullptr, GL_STREAM_DRAW);
}

void Renderer::begin_frame(const float* position, const float zoom) {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  mat3 scaling = { 2.0f * zoom / s_size.x, 0.0f, 0.0f, 0.0f, -2.0f * zoom / s_size.y, 0.0f, 0.0f, 0.0f, 1.0f };
  mat3 translation = { 1.0f, 0.0f, (-s_size.x / zoom + 2 * position[0]) / 2.0f, 0.0f, 1.0f, (-s_size.y / zoom + 2 * position[1]) / 2.0f, 0.0f, 0.0f, 1.0f };

  mat3 view_projection_matrix = scaling * translation;

  s_shaders.set_global_uniform("uViewProjectionMatrix", view_projection_matrix);

  begin_batch();
}

void Renderer::end_frame() {
  end_batch();
  flush();
}


void Renderer::draw(const Geometry& geometry) {
  if (s_data.index_count + geometry.indices.size() >= max_index_count ||
    s_data.vertex_count + geometry.vertices.size() >= max_vertex_count) {
    end_batch();
    flush();
    begin_batch();
  }

  for (size_t i = 0; i < geometry.vertices.size(); i++) {
    const Vertex& vertex = geometry.vertices[i];
    s_data.vertex_buffer_ptr->position = vertex.position;
    s_data.vertex_buffer_ptr++;
  }

  for (size_t i = 0; i < geometry.indices.size(); i++) {
    *(s_data.index_buffer_ptr) = s_data.vertex_count + geometry.indices[i];
    s_data.index_buffer_ptr++;
  }

  s_data.vertex_count += (uint32_t)geometry.vertices.size();
  s_data.index_count += (uint32_t)geometry.indices.size();
}

void Renderer::begin_batch() {
  s_data.vertex_buffer_ptr = s_data.vertex_buffer;
  s_data.index_buffer_ptr = s_data.index_buffer;
}

void Renderer::end_batch() {
  GLsizeiptr vertex_buffer_size = (uint8_t*)s_data.vertex_buffer_ptr - (uint8_t*)s_data.vertex_buffer;
  GLsizeiptr index_buffer_size = (uint8_t*)s_data.index_buffer_ptr - (uint8_t*)s_data.index_buffer;

  glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, s_data.vertex_buffer);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size, s_data.index_buffer);
}

void Renderer::flush() {
  glDrawElements(GL_TRIANGLES, s_data.index_count, GL_UNSIGNED_INT, nullptr);

  s_data.vertex_count = 0;
  s_data.index_count = 0;
}
