/**
 * @file renderer/instances.cpp
 * @brief This file contains the flushing logic of the instanced renderer.
 */

#include "instances.h"

#include "gpu/device.h"
#include "gpu/render_state.h"
#include "gpu/shaders.h"

#include "../utils/assert.h"

namespace graphick::renderer {

void InstancedRenderer::flush(const ivec2 viewport_size, const mat4& vp_matrix, const float zoom)
{
  GK_ASSERT(m_program && m_vertex_array,
            "Program and vertex array must be set through update_shader()!");

  if (m_instances.empty()) {
    return;
  }

  GPU::RenderState render_state = GPU::RenderState{m_program->program,
                                                   &m_vertex_array->vertex_array,
                                                   GPU::Primitive::Triangles,
                                                   irect(ivec2::zero(), viewport_size)};

  render_state.default_blend().no_depth().no_stencil();
  render_state.uniforms = {{m_program->vp_uniform, vp_matrix}, {m_program->zoom_uniform, zoom}};

  for (const std::vector<PrimitiveInstance>& batch : m_instances.instances.batches) {
    m_instances.instance_buffer.upload(batch.data(), batch.size() * sizeof(PrimitiveInstance));

    GPU::Device::draw_arrays_instanced(
        m_instances.vertex_buffer.size / m_instances.vertex_size, batch.size(), render_state);
  }

  m_instances.instances.clear();
}

}  // namespace graphick::renderer
