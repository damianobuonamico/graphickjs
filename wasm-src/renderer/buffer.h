#pragma once

#include "gpu/gpu_data.h"
#include "gpu/device.h"

#include "../utils/uuid.h"

#include <algorithm>

namespace Graphick::Renderer {

  void copy_error();
  void resize_error();
  void overflow_error();
  void free_buffer(uuid id, GPU::BufferTarget target);
  void upload_to_buffer(uuid id, GPU::BufferTarget target, void* data, size_t size);
  uuid allocate_buffer(GPU::BufferTarget target, size_t size, const std::string& tag);
  const GPU::Buffer& get_buffer(GPU::BufferTarget target, uuid id);

  template <typename T>
  class FixedBuffer {
  public:
    FixedBuffer(size_t count) :
      m_allocated_count(count), m_data(new T[count]), m_data_ptr(m_data) {}

    FixedBuffer(const FixedBuffer&) = delete;
    FixedBuffer(FixedBuffer&&) = delete;

    ~FixedBuffer() { delete[] m_data; }

    virtual inline size_t count() const { return m_data_ptr - m_data; }
    inline size_t size() const { return (size_t)((uint8_t*)m_data_ptr - (uint8_t*)m_data); }
    inline size_t available() const { return m_allocated_count - count(); }
    inline T* data() { return m_data; }
    inline T* head() { return m_data_ptr; }

    void copy(const std::vector<T>& data) {
      if (available() < data.size()) {
        copy_error();
        return;
      }

      std::memcpy(m_data_ptr, data.data(), data.size() * sizeof(T));
      m_data_ptr += data.size();
    }

    inline void push(const T& value) {
      *m_data_ptr = value;
      m_data_ptr++;
    }

    void clear() {
      m_data_ptr = m_data;
    }

    T& operator*() { return m_data_ptr; }

    T* operator->() { return m_data_ptr; }

    T* operator++(int) {
      if (m_data_ptr >= m_data + m_allocated_count) {
        overflow_error();
        m_data_ptr = m_data;
        return m_data_ptr;
      }

      return ++m_data_ptr;
    }
  protected:
    size_t m_allocated_count = 0;

    T* m_data = nullptr;
    T* m_data_ptr = nullptr;
  };

  template <typename T>
  class ResizableBuffer {
  public:
    ResizableBuffer(size_t max_count = (size_t)(2 << 18) / sizeof(T)) :
      m_max_count(std::max(max_count, (size_t)1)),
      m_allocated_count(1),
      m_data((T*)malloc(m_allocated_count * sizeof(T))),
      m_data_ptr(m_data) {
      // if (!m_data) throw std::bad_alloc();
    }

    ResizableBuffer(size_t count, size_t max_count) :
      m_max_count(std::max(max_count, (size_t)1)),
      m_allocated_count(std::clamp(count, (size_t)1, m_max_count)),
      m_data((T*)malloc(std::clamp(count, (size_t)1, m_max_count)) * sizeof(T)),
      m_data_ptr(m_data) {}

    ResizableBuffer(const ResizableBuffer&) = delete;
    ResizableBuffer(ResizableBuffer&&) = delete;

    ~ResizableBuffer() { free(m_data); }

    inline size_t count() const { return m_data_ptr - m_data; }
    inline size_t size() const { return (size_t)((uint8_t*)m_data_ptr - (uint8_t*)m_data); }
    inline size_t available() const { return m_allocated_count - count(); }
    inline T* data() const { return m_data; }
    inline T* head() { return m_data_ptr; }

    void push(const T& value) {
      if (m_data_ptr >= m_data + m_allocated_count) {
        return resize(m_allocated_count * 2, true);
      }

      *m_data_ptr = value;
      m_data_ptr++;
    }

    void clear() {
      m_max_effective_count = std::max({ m_max_effective_count, count(), (size_t)1 });
      if (m_max_effective_count > 1000) {
        console::error("something is wrong!", count());
      }
      console::log("max_effective_count", m_max_effective_count);
      m_ticks++;

      if (m_ticks >= 100) {
        size_t new_count = std::min(m_max_count, (size_t)(m_max_effective_count * 1.2));
        console::log("new_count", new_count);

        if (new_count != m_allocated_count) {
          resize(new_count, false);
        }

        m_max_effective_count = 1;
        m_ticks = 0;
      } else {
        m_data_ptr = m_data;
      }

      console::log("allocated_count", m_allocated_count);
    }

    T& operator*() { return *m_data_ptr; }

    T* operator->() { return m_data_ptr; }

    virtual T* operator++(int) {
      if (m_data_ptr >= m_data + m_allocated_count) {
        return resize(m_allocated_count * 2, true);
      }

      return ++m_data_ptr;
    }
  private:
    T* resize(size_t new_count, bool copy_data) {
      size_t target_count = std::min(new_count, m_max_count);
      size_t old_count = count();

      if (target_count == m_allocated_count) {
        resize_error();
        m_data_ptr = m_data;
        return m_data;
      }

      if (copy_data) {
        T* tmp = (T*)realloc(m_data, target_count * sizeof(T));
        // if (!tmp) throw std::bad_alloc();

        m_data = tmp;
        m_data_ptr = m_data + old_count;
      } else {
        free(m_data);

        m_data = (T*)malloc(target_count * sizeof(T));
        // if (!m_data) throw std::bad_alloc();

        m_data_ptr = m_data;
      }

      // if (copy_data) {
      //   T* temp_data = new T[target_count];

      //   size_t count = this->count();
      //   size_t size = this->size();

      //   std::memcpy(temp_data, m_data, size);
      //   delete[] m_data;

      //   m_data = temp_data;
      //   m_data_ptr = m_data + count;
      // } else {
      //   delete[] m_data;
      //   m_data = new T[target_count];
      //   m_data_ptr = m_data;
      // }

      m_allocated_count = target_count;
      return m_data_ptr;
    }
  protected:
    size_t m_ticks = 0;
    size_t m_max_count = 0;
    size_t m_max_effective_count = 0;
    size_t m_allocated_count = 0;

    T* m_data = nullptr;
    T* m_data_ptr = nullptr;
  };

  template <typename T>
  class FixedGPUBuffer : public FixedBuffer<T> {
  public:
    FixedGPUBuffer(std::string tag, size_t count, GPU::BufferTarget target = GPU::BufferTarget::Vertex) :
      FixedBuffer<T>(count), m_tag(tag), m_buffer_target(target) {
      m_buffer_id = allocate_buffer(target, this->m_allocated_count * sizeof(T), tag);
    }

    FixedGPUBuffer(const FixedGPUBuffer&) = delete;
    FixedGPUBuffer(FixedGPUBuffer&&) = delete;

    ~FixedGPUBuffer() {
      free_buffer(m_buffer_id, m_buffer_target);
    }

    inline uuid id() const { return m_buffer_id; }
    const GPU::Buffer& buffer() const { return get_buffer(m_buffer_target, m_buffer_id); }

    void upload() {
      upload_to_buffer(m_buffer_id, m_buffer_target, this->m_data, this->size());
    }
  private:
    std::string m_tag = "";
    uuid m_buffer_id = 0;
    GPU::BufferTarget m_buffer_target = GPU::BufferTarget::Vertex;
  };

  template <typename T>
  class ResizableGPUBuffer : public ResizableBuffer<T> {
  public:
    ResizableGPUBuffer(std::string tag, size_t max_count = (size_t)(2 << 18) / sizeof(T), GPU::BufferTarget target = GPU::BufferTarget::Vertex) :
      ResizableBuffer<T>(max_count), m_tag(tag), m_buffer_target(target), m_buffer_count(max_count) {
      m_buffer_id = allocate_buffer(target, this->m_allocated_count * sizeof(T), tag);
    }

    ResizableGPUBuffer(std::string tag, size_t count, size_t max_count, GPU::BufferTarget target = GPU::BufferTarget::Vertex) :
      ResizableBuffer<T>(count, max_count), m_tag(tag), m_buffer_target(target), m_buffer_count(max_count) {
      m_buffer_id = allocate_buffer(target, this->m_allocated_count * sizeof(T), tag);
    }

    ResizableGPUBuffer(const ResizableGPUBuffer&) = delete;
    ResizableGPUBuffer(ResizableGPUBuffer&&) = delete;

    ~ResizableGPUBuffer() {
      free_buffer(m_buffer_id, m_buffer_target);
    }

    inline uuid id() const { return m_buffer_id; }
    const GPU::Buffer& buffer() const { return get_buffer(m_buffer_target, m_buffer_id); }

    // T* operator++(int) override {
    //   if (m_data_ptr >= m_data + m_allocated_count) {
    //     return resize_and_invalidate(m_allocated_count * 2, true);
    //   }

    //   return ++m_data_ptr;
    // }

    void upload() {
      ensure_buffer_size();
      upload_to_buffer(m_buffer_id, m_buffer_target, this->m_data, this->m_allocated_count * sizeof(T));
    }
  private:
    // T* resize_and_invalidate(size_t new_count, bool copy_data) {
    //   size_t target_count = std::min(new_count, m_max_count);

    //   if (target_count == m_allocated_count) {
    //     resize_error();
    //     m_data_ptr = m_data;
    //     return m_data;
    //   }

    //   if (m_buffer_id) {
    //     free_buffer(m_buffer_id, m_buffer_target);
    //     m_buffer_id = 0;
    //   }

    //   if (copy_data) {
    //     T* temp_data = new T[target_count];

    //     size_t count = this->count();
    //     size_t size = this->size();

    //     //std::memcpy(temp_data, m_data, size);
    //     delete[] m_data;

    //     m_data = temp_data;
    //     m_data_ptr = m_data + count;
    //   } else {
    //     delete[] m_data;
    //     m_data = new T[target_count];
    //     m_data_ptr = m_data;
    //   }

    //   m_allocated_count = target_count;
    //   return m_data_ptr;
    // }

    void ensure_buffer_size() {
      if (this->m_allocated_count == m_buffer_count) return;

      if (m_buffer_id != 0) {
        free_buffer(m_buffer_id, m_buffer_target);
        m_buffer_id = 0;
      }

      m_buffer_id = allocate_buffer(m_buffer_target, this->m_allocated_count * sizeof(T), m_tag);
      m_buffer_count = this->m_allocated_count;
    }
  private:
    std::string m_tag = "";
    uuid m_buffer_id = 0;
    size_t m_buffer_count = 0;
    GPU::BufferTarget m_buffer_target = GPU::BufferTarget::Vertex;
  };

  class GPUUintTexture : public FixedBuffer<uint8_t> {
  public:
    GPUUintTexture(GPU::TextureFormat format, ivec2 size, const std::string& tag);

    inline uuid id() const { return m_texture_id; }
    const GPU::Texture& texture() const;

    inline size_t count() const { return (m_data_ptr - m_data) / GPU::bytes_per_pixel(m_format); }

    void upload();
  private:
    std::string m_tag = "";
    uuid m_texture_id;
    ivec2 m_texture_size;
    GPU::TextureFormat m_format;
  };

  class GPUFloatTexture : public FixedBuffer<float> {
  public:
    GPUFloatTexture(GPU::TextureFormat format, ivec2 size, const std::string& tag);

    inline uuid id() const { return m_texture_id; }
    const GPU::Texture& texture() const;

    inline size_t count() const { return (size_t)((uint8_t*)m_data_ptr - (uint8_t*)m_data) / GPU::bytes_per_pixel(m_format); }

    void push(const float value);
    void push(const vec4& value);
    void upload();
  private:
    std::string m_tag = "";
    uuid m_texture_id;
    ivec2 m_texture_size;
    GPU::TextureFormat m_format;
  };

};
