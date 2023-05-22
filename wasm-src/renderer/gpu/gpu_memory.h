#pragma once

#include "device.h"

#include "../../utils/uuid.h"

#include <chrono>

namespace Graphick::Render::GPU::Memory {

  using time_point = std::chrono::steady_clock::time_point;

  struct TextureDescriptor {
    ivec2 size;
    TextureFormat format;

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

  enum class AllocationKind {
    Buffer,
    Texture,
    Framebuffer
  };

  struct BufferAllocation {
    const AllocationKind kind = AllocationKind::Buffer;

    std::unique_ptr<Buffer> buffer;
    size_t size;
    std::string tag;

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

  struct TextureAllocation {
    const AllocationKind kind = AllocationKind::Texture;

    std::unique_ptr<Texture> texture;
    TextureDescriptor descriptor;
    std::string tag;

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

  struct FramebufferAllocation {
    const AllocationKind kind = AllocationKind::Framebuffer;

    std::unique_ptr<Framebuffer> framebuffer;
    TextureDescriptor descriptor;
    std::string tag;

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

  struct FreeAllocation {
    UUID id;

    FreeAllocation() : id() {}
    FreeAllocation(UUID id) : id(id) {}
  };

  struct FreeGeneralBuffer : public FreeAllocation {
    BufferAllocation allocation;

    FreeGeneralBuffer(BufferAllocation allocation)
      : FreeAllocation(), allocation(allocation) {}
    FreeGeneralBuffer(UUID id, BufferAllocation allocation)
      : FreeAllocation(id), allocation(allocation) {}
  };

  struct FreeIndexBuffer : public FreeAllocation {
    BufferAllocation allocation;

    FreeIndexBuffer(BufferAllocation allocation)
      : FreeAllocation(), allocation(allocation) {}
    FreeIndexBuffer(UUID id, BufferAllocation allocation)
      : FreeAllocation(id), allocation(allocation) {}
  };

  struct FreeTexture : public FreeAllocation {
    TextureAllocation allocation;

    FreeTexture(TextureAllocation allocation)
      : FreeAllocation(), allocation(allocation) {}
    FreeTexture(UUID id, TextureAllocation allocation)
      : FreeAllocation(id), allocation(allocation) {}
  };

  struct FreeFramebuffer : public FreeAllocation {
    FramebufferAllocation allocation;

    FreeFramebuffer(FramebufferAllocation allocation)
      : FreeAllocation(), allocation(allocation) {}
    FreeFramebuffer(UUID id, FramebufferAllocation allocation)
      : FreeAllocation(id), allocation(allocation) {}
  };

  struct FreeObject {
    time_point timestamp;
    FreeAllocation kind;
  };

}
