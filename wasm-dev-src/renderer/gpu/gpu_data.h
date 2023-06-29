#pragma once

#include "../../math/ivec2.h"
#include "../../math/vec2.h"
#include "../../math/vec4.h"
#include "../../math/mat4.h"

#include <cstdint>
#include <variant>
#include <optional>

namespace Graphick::Renderer::GPU {

  /**
   * The version/dialect of OpenGL we should render with.
   */
  enum class DeviceVersion {
    // OpenGL 3.0+, core profile.
    GL3 = 0,
    // OpenGL ES 3.0+.
    GLES3 = 1,
    // Other backend versions go here.
  };

  enum class ImageAccess {
    Read,
    Write,
    ReadWrite
  };

  enum class TextureFormat {
    R8,
    RGBA8,
    R32F,
    RGBA32F
  };

  enum class TextureSamplingFlag {
    None = 0,
    RepeatU = 1 << 0,
    RepeatV = 1 << 1,
    NearestMin = 1 << 2,
    NearestMag = 1 << 3
  };

  enum class VertexAttrType {
    F32,
    I8,
    I16,
    I32,
    U8,
    U16
  };

  enum class VertexAttrClass {
    Float,
    FloatNorm,
    Int,
  };

  enum class BufferTarget {
    Vertex,
    Index,
    Storage,
  };

  enum class BufferUploadMode {
    Static,
    Dynamic,
    Stream
  };

  enum class ShaderKind {
    Vertex,
    Fragment,
  };

  enum class BlendFactor {
    Zero,
    One,
    SrcAlpha,
    OneMinusSrcAlpha,
    DestAlpha,
    OneMinusDestAlpha,
    DestColor
  };

  enum class BlendOp {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
  };

  enum class DepthFunc {
    Less,
    Always
  };

  enum class StencilFunc {
    Always,
    Equal
  };

  enum class Primitive {
    Triangles,
    Lines,
  };

  struct TextureData {
    size_t size;

    TextureData(size_t size) : size(size) {}
  };

  struct U8TextureData : public TextureData {
    uint8_t* data;

    U8TextureData(size_t width, size_t height, uint8_t channels)
      : TextureData(width* height* channels), data(new uint8_t[size]) {}
    ~U8TextureData() { delete[] data; }
  };

  struct F32TextureData : public TextureData {
    float* data;

    F32TextureData(size_t width, size_t height, uint8_t channels)
      : TextureData(width* height* channels), data(new float[size]) {}
    ~F32TextureData() { delete[] data; }
  };

  struct VertexAttrDescriptor {
    size_t size;
    VertexAttrClass attr_class;
    VertexAttrType attr_type;
    size_t stride;
    size_t offset;
    uint32_t divisor;
    uint32_t buffer_index;
  };

  using UniformData = std::variant<
    int,
    ivec2,
    float,
    vec2,
    vec4,
    mat4
  >;

  struct ClearOps {
    // TODO: Replace with Color class
    std::optional<vec4> color;
    std::optional<float> depth;
    std::optional<uint8_t> stencil;

    inline bool has_ops() const { return color.has_value() || depth.has_value() || stencil.has_value(); }
  };

  struct BlendState {
    BlendFactor src_rgb_factor;
    BlendFactor dest_rgb_factor;
    BlendFactor src_alpha_factor;
    BlendFactor dest_alpha_factor;
    BlendOp op;
  };

  struct DepthState {
    DepthFunc func;
    bool write;
  };

  struct StencilState {
    StencilFunc func;
    uint32_t reference;
    uint32_t mask;
    bool write;
  };

  struct RenderOptions {
    std::optional<BlendState> blend;
    std::optional<DepthState> depth;
    std::optional<StencilState> stencil;
    ClearOps clear_ops;
    bool color_mask;
  };

  template <typename U>
  using UniformBinding = std::pair<U, UniformData>;

  template <typename TP, typename T>
  using TextureBinding = std::pair<TP, T>;

  constexpr uint8_t bytes_per_pixel(TextureFormat format) {
    switch (format) {
    case TextureFormat::R8: return 1;
    case TextureFormat::RGBA8:
    case TextureFormat::R32F: return 4;
    default:
    case TextureFormat::RGBA32F: return 16;
    }
  }

  constexpr uint8_t channels_per_pixel(TextureFormat format) {
    switch (format) {
    case TextureFormat::R8:
    case TextureFormat::R32F: return 1;
    default:
    case TextureFormat::RGBA8:
    case TextureFormat::RGBA32F: return 4;
    }
  }

}
