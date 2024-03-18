/**
 * @file gpu_memory.h
 * @brief Contains the GPU memory allocation definitions.
 */

#pragma once

#include "device.h"

#include "../../utils/uuid.h"

#include <chrono>

namespace Graphick::Renderer::GPU::Memory {

  using time_point = std::chrono::steady_clock::time_point;

  /**
   * @brief The texture metadata.
   *
   * @struct TextureDescriptor
   */
  struct TextureDescriptor {
    ivec2 size;              /* The texture size. */
    TextureFormat format;    /* The texture format. */

    bool operator==(const TextureDescriptor& other) const {
      return size == other.size && format == other.format;
    }
    bool operator!=(const TextureDescriptor& other) const {
      return !(*this == other);
    }

    inline size_t byte_size() const {
      return (size_t)size.x * (size_t)size.y * (size_t)bytes_per_pixel(format);
    }
  };

  /**
   * @brief The GPU memory allocation kind.
   */
  enum class AllocationKind {
    Buffer,
    IndexBuffer,
    Texture,
    Framebuffer
  };

  /**
   * @brief A buffer allocation.
   *
   * @struct BufferAllocation
   */
  struct BufferAllocation {
    const AllocationKind kind = AllocationKind::Buffer;    /* The allocation kind. */

    std::unique_ptr<Buffer> buffer;                        /* The underlying buffer. */
    size_t size;                                           /* The buffer size. */
    std::string tag;                                       /* The allocation tag. */

    /**
     * @brief Get the underlying buffer.
     *
     * @return The underlying buffer.
     */
    const Buffer& get() const { return *buffer; }

    BufferAllocation() : buffer(nullptr), size(0), tag() {}
    BufferAllocation(std::unique_ptr<Buffer> buffer, size_t size, std::string tag)
      : buffer(std::move(buffer)), size(size), tag(tag) {}
    BufferAllocation(BufferAllocation& other) noexcept
      : buffer(std::move(other.buffer)), size(other.size), tag(other.tag) {}
    BufferAllocation(BufferAllocation&& other) noexcept
      : buffer(std::move(other.buffer)), size(other.size), tag(other.tag) {}
    BufferAllocation(const BufferAllocation& other) = delete;
  };

  /**
   * @brief A texture allocation.
   *
   * @struct TextureAllocation
  */
  struct TextureAllocation {
    const AllocationKind kind = AllocationKind::Texture;    /* The allocation kind. */

    std::unique_ptr<Texture> texture;                       /* The underlying texture. */
    TextureDescriptor descriptor;                           /* The texture descriptor. */
    std::string tag;                                        /* The allocation tag. */

    /**
     * @brief Get the underlying texture.
     *
     * @return The underlying texture.
     */
    const Texture& get() const { return *texture; }

    TextureAllocation() : texture(nullptr), descriptor(), tag() {}
    TextureAllocation(std::unique_ptr<Texture> texture, TextureDescriptor descriptor, std::string tag)
      : texture(std::move(texture)), descriptor(descriptor), tag(tag) {}
    TextureAllocation(TextureAllocation& other) noexcept
      : texture(std::move(other.texture)), descriptor(other.descriptor), tag(other.tag) {}
    TextureAllocation(TextureAllocation&& other) noexcept
      : texture(std::move(other.texture)), descriptor(other.descriptor), tag(other.tag) {}
    TextureAllocation(const TextureAllocation& other) = delete;
  };

  /**
   * @brief A framebuffer allocation.
   *
   * @struct FramebufferAllocation
   */
  struct FramebufferAllocation {
    const AllocationKind kind = AllocationKind::Framebuffer;    /* The allocation kind. */

    std::unique_ptr<Framebuffer> framebuffer;                   /* The underlying framebuffer. */
    TextureDescriptor descriptor;                               /* The framebuffer descriptor. */
    std::string tag;                                            /* The allocation tag. */

    /**
     * @brief Get the underlying framebuffer.
     *
     * @return The underlying framebuffer.
     */
    const Framebuffer& get() const { return *framebuffer; }

    FramebufferAllocation() : framebuffer(nullptr), descriptor(), tag() {}
    FramebufferAllocation(std::unique_ptr<Framebuffer> framebuffer, TextureDescriptor descriptor, std::string tag)
      : framebuffer(std::move(framebuffer)), descriptor(descriptor), tag(tag) {}
    FramebufferAllocation(FramebufferAllocation& other) noexcept
      : framebuffer(std::move(other.framebuffer)), descriptor(other.descriptor), tag(other.tag) {}
    FramebufferAllocation(FramebufferAllocation&& other) noexcept
      : framebuffer(std::move(other.framebuffer)), descriptor(other.descriptor), tag(other.tag) {}
    FramebufferAllocation(const FramebufferAllocation& other) = delete;
  };

  /**
   * @brief A free allocation.
   *
   * When an allocation is freed, it is added to the free list.
   * A free allocation can be permanently removed from memory (and the free list) or reused.
   *
   * @struct FreeAllocation
   */
  struct FreeAllocation {
    uuid id;                /* The allocation UUID. */
    AllocationKind kind;    /* The allocation kind. */

    FreeAllocation(AllocationKind kind) : id(), kind(kind) {}
    FreeAllocation(AllocationKind kind, uuid id) : id(id), kind(kind) {}
  };

  /**
   * @brief A free buffer allocation.
   *
   * @struct FreeGeneralBuffer
   */
  struct FreeGeneralBuffer : public FreeAllocation {
    BufferAllocation allocation;    /* The buffer allocation. */

    FreeGeneralBuffer(BufferAllocation allocation)
      : FreeAllocation(AllocationKind::Buffer), allocation(allocation) {}
    FreeGeneralBuffer(uuid id, BufferAllocation allocation)
      : FreeAllocation(AllocationKind::Buffer, id), allocation(allocation) {}
  };

  /**
   * @brief A free index buffer allocation.
   *
   * @struct FreeIndexBuffer
   */
  struct FreeIndexBuffer : public FreeAllocation {
    BufferAllocation allocation;    /* The buffer allocation. */

    FreeIndexBuffer(BufferAllocation allocation)
      : FreeAllocation(AllocationKind::IndexBuffer), allocation(allocation) {}
    FreeIndexBuffer(uuid id, BufferAllocation allocation)
      : FreeAllocation(AllocationKind::IndexBuffer, id), allocation(allocation) {}
  };

  /**
   * @brief A free texture allocation.
   *
   * @struct FreeTexture
   */
  struct FreeTexture : public FreeAllocation {
    TextureAllocation allocation;    /* The texture allocation. */

    FreeTexture(TextureAllocation allocation)
      : FreeAllocation(AllocationKind::Texture), allocation(allocation) {}
    FreeTexture(uuid id, TextureAllocation allocation)
      : FreeAllocation(AllocationKind::Texture, id), allocation(allocation) {}
  };

  /**
   * @brief A free framebuffer allocation.
   *
   * @struct FreeFramebuffer
   */
  struct FreeFramebuffer : public FreeAllocation {
    FramebufferAllocation allocation;    /* The framebuffer allocation. */

    FreeFramebuffer(FramebufferAllocation allocation)
      : FreeAllocation(AllocationKind::Framebuffer), allocation(allocation) {}
    FreeFramebuffer(uuid id, FramebufferAllocation allocation)
      : FreeAllocation(AllocationKind::Framebuffer, id), allocation(allocation) {}
  };

  /**
   * @brief A free object.
   *
   * @struct FreeObject
   */
  struct FreeObject {
    time_point timestamp;    /* When the object was freed. */
    FreeAllocation kind;     /* The free allocation kind. */
  };

}
