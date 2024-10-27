/**
 * @file renderer/gpu/gpu_data.h
 * @brief Contains the GPU data definitions.
 */

#pragma once

#include "../../math/mat4.h"
#include "../../math/vec2.h"
#include "../../math/vec4.h"

#include <optional>
#include <variant>
#include <vector>

namespace graphick::renderer::GPU {

/**
 * @brief The version/dialect of OpenGL we should render with.
 */
enum class DeviceVersion {
  GL3 = 0,    // OpenGL 3.0+, core profile.
  GLES3 = 1,  // OpenGL ES 3.0+.
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
 * @brief The color blend factor.
 */
enum class BlendFactor { Zero, One, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha, DestColor };

/**
 * @brief The color blend operation.
 */
enum class BlendOp { Add, Subtract, ReverseSubtract, Min, Max };

/**
 * @brief The depth function.
 */
enum class DepthFunc { Less, Lequal, Always };

/**
 * @brief The stencil function.
 */
enum class StencilFunc { Always, Equal, Nequal };

/**
 * @brief The vertex attribute type.
 */
enum class VertexAttrType { F16, F32, I8, I16, I32, U8, U16, U32 };

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
enum class TextureFormat { R8, R16UI, RGBA8, RGBA8UI, R16F, R32F, RGBA16F, RGBA32F };

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
enum class BufferUploadMode { Static, Dynamic, Stream };

/**
 * @brief The vertex attribute descriptor.
 *
 * @struct VertexAttrDescriptor
 */
struct VertexAttrDescriptor {
  VertexAttrClass attr_class;  // The attribute class.
  VertexAttrType attr_type;    // The attribute type.

  uint32_t size;               // The size of the attribute.
  uint32_t stride;             // The stride of the attribute.
  uint32_t offset;             // The offset of the attribute.
  uint32_t divisor;            // The divisor of the attribute.
  uint32_t buffer_index;       // The buffer index of the attribute.
};

/**
 * @brief The blend state.
 *
 * @struct BlendState
 */
struct BlendState {
  BlendFactor src_rgb_factor;     // The source RGB factor.
  BlendFactor dest_rgb_factor;    // The destination RGB factor.
  BlendFactor src_alpha_factor;   // The source alpha factor.
  BlendFactor dest_alpha_factor;  // The destination alpha factor.
  BlendOp op;                     // The blend operation.

  bool operator==(const BlendState& other) const {
    return src_rgb_factor == other.src_rgb_factor && dest_rgb_factor == other.dest_rgb_factor &&
      src_alpha_factor == other.src_alpha_factor && dest_alpha_factor == other.dest_alpha_factor && op == other.op;
  }

  bool operator!=(const BlendState& other) const { return !operator==(other); }
};

/**
 * @brief The depth state.
 *
 * @struct DepthState
 */
struct DepthState {
  DepthFunc func;  // The depth function.
  bool write;      // Whether to write to the depth buffer.

  bool operator==(const DepthState& other) const { return func == other.func && write == other.write; }

  bool operator!=(const DepthState& other) const { return !operator==(other); }
};

/**
 * @brief The stencil state.
 *
 * @struct StencilState
 */
struct StencilState {
  StencilFunc func;    // The stencil function.
  uint32_t reference;  // The reference value.
  uint32_t mask;       // The mask value.
  bool write;          // Whether to write to the stencil buffer.

  bool operator==(const StencilState& other) const {
    return func == other.func && reference == other.reference && mask == other.mask && write == other.write;
  }

  bool operator!=(const StencilState& other) const { return !operator==(other); }
};

/**
 * @brief The clear operations.
 *
 * @struct ClearOps
 */
struct ClearOps {
  std::optional<vec4> color;       // The color clear value. If std::nullopt, the color buffer is not cleared.
  std::optional<float> depth;      // The depth clear value. If std::nullopt, the depth buffer is not cleared.
  std::optional<uint8_t> stencil;  // The stencil clear value. If std::nullopt, the stencil buffer is not cleared.
};

/**
 * @brief The uniform data.
 */
using UniformData = std::variant<uint16_t, uint32_t, int, float, ivec2, vec2, vec4, mat4, std::vector<int>, std::vector<vec4>>;

}  // namespace graphick::renderer::GPU
