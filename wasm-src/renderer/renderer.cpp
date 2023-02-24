#include "renderer.h"

#include "../common.h"

#ifdef __EMSCRIPTEN__
#define EMSCRIPTEN
#endif

#ifdef EMSCRIPTEN
#include <GLES3/gl32.h>
#include <emscripten/html5.h>
#else
#include <glad/glad.h>
#endif

#include <stdexcept>

Renderer* Renderer::s_instance = nullptr;

void Renderer::init() {
  assert(!s_instance);
  s_instance = new Renderer();

  const RendererSettings& settings = get()->m_settings;

#ifdef EMSCRIPTEN
  EmscriptenWebGLContextAttributes attr;

  attr.alpha = true;
  attr.premultipliedAlpha = true;
  attr.majorVersion = 2;
  attr.antialias = settings.antialiasing == Antialiasing::Hardware;

  emscripten_webgl_init_context_attributes(&attr);

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);
#else
  if (settings.antialiasing == Antialiasing::Hardware) {
    glEnable(GL_MULTISAMPLE);
  }
#endif

  glLineWidth(1.0f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

void Renderer::resize(const vec2& size) {
  get()->m_size = size;
  get()->m_frame_buffer.resize(size);
  glViewport(0, 0, (GLsizei)size.x, (GLsizei)size.y);
}

void Renderer::begin_frame(const vec2& position, float zoom) {
  get()->set_viewport(position, zoom);
  get()->m_frame_buffer.bind();

  glClearColor(0.09f, 0.11f, 0.13f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  get()->begin_batch();
}

void Renderer::end_frame() {
  if (get()->m_last_call == RenderCall::Batch) {
    get()->end_batch();
    get()->flush();
  }

  get()->m_last_call = RenderCall::None;
  get()->m_frame_buffer.render();
}

void Renderer::push_overlay_layer(const vec2& position) {
  if (get()->m_last_call == RenderCall::Batch) {
    get()->end_batch();
    get()->flush();
  }

  get()->m_last_call = RenderCall::None;

  get()->set_viewport(position, 1.0f);
  get()->begin_batch();
}

void Renderer::draw(const Geometry& geometry) {
  if (get()->m_last_call != RenderCall::Batch) {
    get()->bind_batch_renderer();
    get()->m_last_call = RenderCall::Batch;
  }

  get()->add_to_batch(geometry);
};

void Renderer::draw(const InstancedGeometry& geometry) {
  if (geometry.instances() == 0) {
    return;
  }

  get()->bind_instance_renderer();
  get()->m_last_call = RenderCall::Instance;

  get()->draw_instanced(geometry);
}

Renderer::Renderer(): m_frame_buffer(m_shaders, m_size) {}

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
  mat3 scaling = { 2.0f * zoom / m_size.x, 0.0f, 0.0f, 0.0f, -2.0f * zoom / m_size.y, 0.0f, 0.0f, 0.0f, 1.0f };
  mat3 translation = { 1.0f, 0.0f, (-m_size.x / zoom + 2 * position[0]) / 2.0f, 0.0f, 1.0f, (-m_size.y / zoom + 2 * position[1]) / 2.0f, 0.0f, 0.0f, 1.0f };

  mat3 view_projection_matrix = scaling * translation;

  m_shaders.set_view_projection_matrix(view_projection_matrix);
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

  glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, m_data.vertex_buffer);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size, m_data.index_buffer);
}

void Renderer::flush() {
  if (m_data.vertex_count == 0 || m_data.index_count == 0) {
    return;
  }

  glDrawElements(m_data.primitive, m_data.index_count, GL_UNSIGNED_INT, nullptr);

  m_data.vertex_count = 0;
  m_data.index_count = 0;
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
