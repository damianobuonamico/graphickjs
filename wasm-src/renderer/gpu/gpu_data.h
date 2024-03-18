/**
 * @file gpu_data.h
 * @brief Contains the GPU data definitions.
 */

#pragma once

#include "../../math/vec2.h"
#include "../../math/vec4.h"
#include "../../math/mat4.h"

#include <cstdint>
#include <variant>
#include <optional>

namespace Graphick::Renderer::GPU {

  /**
   * @brief The version/dialect of OpenGL we should render with.
   */
  enum class DeviceVersion {
    GL3 = 0,      /* OpenGL 3.0+, core profile. */
    GLES3 = 1,    /* OpenGL ES 3.0+. */
  };

  /**
   * @brief The kind of access to an image.
   */
  enum class ImageAccess {
    Read,
    Write,
    ReadWrite
  };

  /**
   * @brief The texture format.
   */
  enum class TextureFormat {
    R8,
    RGBA8,
    RGBA8UI,
    R32F,
    RGBA32F
  };

  /**
   * @brief The texture sampling flags.
   */
  enum class TextureSamplingFlag {
    None = 0,
    RepeatU = 1 << 0,
    RepeatV = 1 << 1,
    NearestMin = 1 << 2,
    NearestMag = 1 << 3
  };

  /**
   * @brief The vertex attribute type.
   */
  enum class VertexAttrType {
    F32,
    I8,
    I16,
    I32,
    U8,
    U16
  };

  /**
   * @brief The vertex attribute class.
   */
  enum class VertexAttrClass {
    Float,
    FloatNorm,
    Int,
  };

  /**
   * @brief The buffer target.
   */
  enum class BufferTarget {
    Vertex,
    Index,
    Storage,
  };

  /**
   * @brief The buffer upload mode.
   */
  enum class BufferUploadMode {
    Static,
    Dynamic,
    Stream
  };

  /**
   * @brief The shader kind.
   */
  enum class ShaderKind {
    Vertex,
    Fragment,
  };

  /**
   * @brief The color blend factor.
   */
  enum class BlendFactor {
    Zero,
    One,
    SrcAlpha,
    OneMinusSrcAlpha,
    DestAlpha,
    OneMinusDestAlpha,
    DestColor
  };

  /**
   * @brief The color blend operation.
   */
  enum class BlendOp {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
  };

  /**
   * @brief The depth function.
   */
  enum class DepthFunc {
    Less,
    Lequal,
    Always
  };

  /**
   * @brief The stencil function.
   */
  enum class StencilFunc {
    Always,
    Equal
  };

  /**
   * @brief The primitive type.
   */
  enum class Primitive {
    Triangles,
    Lines,
  };

  /**
   * @brief The texture data.
   *
   * @struct TextureData
   */
  struct TextureData {
    size_t size;    /* The size of the texture data. */

    TextureData(size_t size) : size(size) {}
  };

  /**
   * @brief The 8-bit texture data.
   *
   * @struct U8TextureData
  */
  struct U8TextureData : public TextureData {
    uint8_t* data;    /* The underlying texture data. */

    U8TextureData(size_t width, size_t height, uint8_t channels)
      : TextureData(width* height* channels), data(new uint8_t[size]) {}
    ~U8TextureData() { delete[] data; }
  };

  /**
   * @brief The 32-bit float texture data.
   *
   * @struct F32TextureData
   */
  struct F32TextureData : public TextureData {
    float* data;    /* The underlying texture data. */

    F32TextureData(size_t width, size_t height, uint8_t channels)
      : TextureData(width* height* channels), data(new float[size]) {}
    ~F32TextureData() { delete[] data; }
  };

  /**
   * @brief The vertex attribute descriptor.
   *
   * @struct VertexAttrDescriptor
   */
  struct VertexAttrDescriptor {
    size_t size;                   /* The size of the attribute. */
    VertexAttrClass attr_class;    /* The attribute class. */
    VertexAttrType attr_type;      /* The attribute type. */
    size_t stride;                 /* The stride of the attribute. */
    size_t offset;                 /* The offset of the attribute. */
    uint32_t divisor;              /* The divisor of the attribute. */
    uint32_t buffer_index;         /* The buffer index of the attribute. */
  };

  using UniformData = std::variant<
    int,
    ivec2,
    float,
    vec2,
    vec4,
    mat4
  >;

  /**
   * @brief The clear operations.
   */
  struct ClearOps {
    std::optional<vec4> color;         /* The color clear value, if std::nullopt, the color buffer is not cleared. */
    std::optional<float> depth;        /* The depth clear value, if std::nullopt, the depth buffer is not cleared. */
    std::optional<uint8_t> stencil;    /* The stencil clear value, if std::nullopt, the stencil buffer is not cleared. */

    /**
     * @brief Check if the clear operations are not empty.
     *
     * @return true if the clear operations are not empty, false otherwise.
     */
    inline bool has_ops() const { return color.has_value() || depth.has_value() || stencil.has_value(); }
  };

  /**
   * @brief The blend state.
   *
   * @struct BlendState
   */
  struct BlendState {
    BlendFactor src_rgb_factor;       /* The source RGB factor. */
    BlendFactor dest_rgb_factor;      /* The destination RGB factor. */
    BlendFactor src_alpha_factor;     /* The source alpha factor. */
    BlendFactor dest_alpha_factor;    /* The destination alpha factor. */
    BlendOp op;                       /* The blend operation. */
  };

  /**
   * @brief The depth state.
   *
   * @struct DepthState
   */
  struct DepthState {
    DepthFunc func;    /* The depth function. */
    bool write;        /* Whether to write to the depth buffer. */
  };

  /**
   * @brief The stencil state.
   *
   * @struct StencilState
   */
  struct StencilState {
    StencilFunc func;      /* The stencil function. */
    uint32_t reference;    /* The reference value. */
    uint32_t mask;         /* The mask value. */
    bool write;            /* Whether to write to the stencil buffer. */
  };

  /**
   * @brief The render options.
   *
   * @struct RenderOptions
   */
  struct RenderOptions {
    std::optional<BlendState> blend;        /* The blend state, if std::nullopt, blending is disabled. */
    std::optional<DepthState> depth;        /* The depth state, if std::nullopt, the depth test is disabled. */
    std::optional<StencilState> stencil;    /* The stencil state, if std::nullopt, the stencil test is disabled. */
    ClearOps clear_ops;                     /* The clear operations. */
    bool color_mask;                        /* Whether to write to the color buffer. */
  };

  template <typename U>
  using UniformBinding = std::pair<U, UniformData>;

  template <typename TP, typename T>
  using TextureBinding = std::pair<TP, T>;

  /**
   * @brief Returns the number of bytes per pixel for the given texture format.
   *
   * @param format The texture format.
   * @return The number of bytes per pixel.
   */
  constexpr uint8_t bytes_per_pixel(TextureFormat format) {
    switch (format) {
    case TextureFormat::R8: return 1;
    case TextureFormat::RGBA8:
    case TextureFormat::R32F: return 4;
    default:
    case TextureFormat::RGBA32F: return 16;
    }
  }

  /**
   * @brief Returns the number of channels per pixel for the given texture format.
   *
   * @param format The texture format.
   * @return The number of channels per pixel.
   */
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
