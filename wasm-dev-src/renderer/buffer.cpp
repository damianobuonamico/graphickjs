#include "buffer.h"

#include "gpu/allocator.h"

#include "../utils/console.h"

namespace Graphick::Renderer {

  void copy_error() {
    console::error("FixedBuffer::copy: Not enough available space to hold data!");
  }

  void resize_error() {
    console::error("ResizableBuffer::resize: Cannot resize buffer to same size!");
  }

  void overflow_error() {
    console::error("FixedBuffer::operator++: Buffer overflow!");
  }

  void free_buffer(uuid id, GPU::BufferTarget target) {
    if (target == GPU::BufferTarget::Vertex) {
      GPU::Memory::Allocator::free_general_buffer(id);
    } else {
      GPU::Memory::Allocator::free_index_buffer(id);
    }
  }

  void upload_to_buffer(uuid id, GPU::BufferTarget target, void* data, size_t size) {
    if (target == GPU::BufferTarget::Vertex) {
      const GPU::Buffer& buffer = GPU::Memory::Allocator::get_general_buffer(id);
      GPU::Device::upload_to_buffer(buffer, 0, data, size, target);
    } else {
      const GPU::Buffer& buffer = GPU::Memory::Allocator::get_index_buffer(id);
      GPU::Device::upload_to_buffer(buffer, 0, data, size, target);
    }
  }

  uuid allocate_buffer(GPU::BufferTarget target, size_t size, const std::string& tag) {
    if (target == GPU::BufferTarget::Vertex) {
      return GPU::Memory::Allocator::allocate_byte_general_buffer(size, tag);
    } else {
      return GPU::Memory::Allocator::allocate_byte_index_buffer(size, tag);
    }
  }

  const GPU::Buffer& get_buffer(GPU::BufferTarget target, uuid id) {
    if (target == GPU::BufferTarget::Vertex) {
      return GPU::Memory::Allocator::get_general_buffer(id);
    } else {
      return GPU::Memory::Allocator::get_index_buffer(id);
    }
  }

  GPUUintTexture::GPUUintTexture(GPU::TextureFormat format, ivec2 size, const std::string& tag)
    : FixedBuffer<uint8_t>{ (size_t)(size.x * size.y * GPU::channels_per_pixel(format)) }, m_format(format), m_texture_size(size), m_tag(tag) {
    m_texture_id = GPU::Memory::Allocator::allocate_texture(size, format, tag);
  }

  void GPUUintTexture::upload() {
    const GPU::Texture& texture = GPU::Memory::Allocator::get_texture(m_texture_id);
    GPU::Device::upload_to_texture(texture, { { 0.0f, 0.0f }, { (float)m_texture_size.x, (float)m_texture_size.y } }, m_data);
  }

  const GPU::Texture& GPUUintTexture::texture() const {
    return GPU::Memory::Allocator::get_texture(m_texture_id);
  }

  GPUFloatTexture::GPUFloatTexture(GPU::TextureFormat format, ivec2 size, const std::string& tag)
    : FixedBuffer<float>{ (size_t)(size.x * size.y * GPU::channels_per_pixel(format)) }, m_format(format), m_texture_size(size), m_tag(tag) {
    m_texture_id = GPU::Memory::Allocator::allocate_texture(size, format, tag);
  }

  void GPUFloatTexture::push(const float value) {
    *m_data_ptr = value;
    m_data_ptr++;
  }

  void GPUFloatTexture::push(const vec4& value) {
    m_data_ptr[0] = value.r;
    m_data_ptr[1] = value.g;
    m_data_ptr[2] = value.b;
    m_data_ptr[3] = value.a;
    m_data_ptr += 4;
  }

  void GPUFloatTexture::upload() {
    const GPU::Texture& texture = GPU::Memory::Allocator::get_texture(m_texture_id);
    GPU::Device::upload_to_texture(texture, { { 0.0f, 0.0f }, { (float)m_texture_size.x, (float)m_texture_size.y } }, m_data);
  }

  const GPU::Texture& GPUFloatTexture::texture() const {
    return GPU::Memory::Allocator::get_texture(m_texture_id);
  }

}
