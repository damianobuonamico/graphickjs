/**
 * @file renderer/gpu/gpu_data.h
 * @brief Contains the GPU data definitions.
 */

#pragma once

#include "../../../math/vec2.h"
#include "../../../math/vec4.h"

#include <optional>

namespace graphick::renderer::GPU {

  /**
   * @brief The version/dialect of OpenGL we should render with.
   */
  enum class DeviceVersion {
    GL3 = 0,      /* OpenGL 3.0+, core profile. */
    GLES3 = 1,    /* OpenGL ES 3.0+. */
  };

  /**
   * @brief The primitive type.
   */
  enum class Primitive {
    Triangles,
    Lines,
  };

  /**
   * @brief The shader kind.
   */
  enum class ShaderKind {
    Vertex,
    Fragment,
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
    U16,
    U32
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
   * @brief The texture format.
   */
  enum class TextureFormat {
    R8,
    R16UI,
    RGBA8,
    RGBA8UI,
    R16F,
    R32F,
    RGBA16F,
    RGBA32F
  };

  /**
   * @brief The texture sampling flags.
   */
  enum TextureSamplingFlag {
    TextureSamplingFlagNone = 0,
    TextureSamplingFlagRepeatU = 1 << 0,
    TextureSamplingFlagRepeatV = 1 << 1,
    TextureSamplingFlagNearestMin = 1 << 2,
    TextureSamplingFlagNearestMag = 1 << 3
  };

  /**
   * @brief The buffer target.
   */
  enum class BufferTarget {
    Vertex,
    Index,
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
   * @brief The vertex attribute descriptor.
   *
   * @struct VertexAttrDescriptor
   */
  struct VertexAttrDescriptor {
    VertexAttrClass attr_class;    /* The attribute class. */
    VertexAttrType attr_type;      /* The attribute type. */

    uint32_t size;                 /* The size of the attribute. */
    uint32_t stride;               /* The stride of the attribute. */
    uint32_t offset;               /* The offset of the attribute. */
    uint32_t divisor;              /* The divisor of the attribute. */
    uint32_t buffer_index;         /* The buffer index of the attribute. */
  };

  /**
   * @brief The clear operations.
   *
   * @struct ClearOps
   */
  struct ClearOps {
    std::optional<vec4> color;         /* The color clear value. If std::nullopt, the color buffer is not cleared. */
    std::optional<float> depth;        /* The depth clear value. If std::nullopt, the depth buffer is not cleared. */
    std::optional<uint8_t> stencil;    /* The stencil clear value. If std::nullopt, the stencil buffer is not cleared. */
  };

}
