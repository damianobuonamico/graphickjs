/**
 * @file renderer_data.h
 * @brief The file contains data structures used by the renderer.
 *
 * @todo maybe replace vectors with preallocated arrays
 */

#pragma once

#include "gpu/render_state.h"

#include "../math/mat2x3.h"
#include "../math/rect.h"
#include "../math/scalar.h"
#include "../math/vec4.h"

#include "../utils/defines.h"
#include "../utils/uuid.h"

namespace graphick::editor {
class Cache;
}

namespace graphick::renderer {

/**
 * @brief Represents the viewport of the renderer.
 *
 * The viewport is the area of the screen where the renderer will draw.
 *
 * @struct Viewport
 */
struct Viewport {
  ivec2 size;      /* The size of the viewport. */
  vec2 position;   /* The position of the viewport. */

  double zoom;     /* The zoom level of the viewport (it is pre-multiplied by the dpr). */
  double dpr;      /* The device pixel ratio. */

  vec4 background; /* The background color to clear the viewport with. */

  /**
   * @brief Default constructor.
   */
  Viewport() = default;

  /**
   * @brief Constructs a new Viewport object.
   *
   * @param size The size of the viewport.
   * @param position The position of the viewport.
   * @param zoom The zoom level of the viewport, pre-multiplied by the dpr.
   * @param dpr The device-pixel-ratio of the screen.
   * @param background The clear color of the viewport.
   */
  Viewport(const ivec2 size, const vec2 position, const double zoom, const double dpr, const vec4& background) :
    size(size),
    position(position),
    zoom(zoom),
    dpr(dpr),
    background(background),
    m_visible({-position, vec2(size) / static_cast<float>(zoom) - position}) { }

  /**
   * @brief Returns the scene-space visible area.
   *
   * @return The scene-space rectangle that is visible in the viewport.
   */
  inline rect visible() const { return m_visible; }

  /**
   * @brief Converts a point from client-space to scene-space.
   *
   * @param p The point in client-space.
   * @return The point in scene-space.
   */
  inline vec2 project(const vec2 p) const { return p / static_cast<float>(zoom) - position; }
private:
  rect m_visible; /* The visible area of the viewport in scene-space coordinates. */
};

/**
 * @brief Represents the options used to render the scene.
 *
 * @struct RenderOptions
 */
struct RenderOptions {
  Viewport viewport;    /* The viewport to render to. */
  editor::Cache* cache; /* The cache to use for rendering, can be nullptr. */

  bool ignore_cache;    /* Whether to ignore the cache and redraw everything. */
};

/**
 * @brief Generates the vertices of a quad.
 *
 * @param min The minimum point of the quad.
 * @param max The maximum point of the quad.
 * @return The vertices of the quad.
 */
inline std::vector<uvec2> quad_vertices(const uvec2 min, const uvec2 max) {
  return {min, uvec2{max.x, min.y}, max, max, uvec2{min.x, max.y}, min};
}

/**
 * @brief Represents a path instance (32 bytes), the main building block of the renderer.
 *
 * @struct PathInstance
 * @note There are 5 bits of padding left in the struct.
 * @note The maximum number of bands is currently 256 here and 32 in the renderer, so a few bits are wasted.
 */
struct PathInstance {
  vec2 position;   /* | position.x (32) | position.y (32) | */
  vec2 size;       /* | size.x (32) | size.y (32) |*/
  uvec4 color;     /* | color.rgba (32) | */
  uint32_t attr_1; /* | (5) - is_cull (1) - is_quad (1) - is_eodd (1) - curves_x (12) - curves_y (12) | */
  uint32_t attr_2; /* | bands_h (8) - bands_x (12) - bands_y (12) | */
  uint32_t attr_3; /* | z_index (20) - transform_index (12) | */

  /**
   * @brief Constructs a new PathInstance object.
   *
   * @param position The position of the path's bounding rect before transformation, used to normalize the curves.
   * @param size The size of the path's bounding rect before transformation, used to normalize the curves.
   * @param color The color of the path.
   * @param curves_start_index The index of the first curve in the curves texture.
   * @param bands_start_index The index of the first band in the bands texture.
   * @param horizontal_bands The number of horizontal bands.
   * @param is_quadratic Whether the curves are quadratic or cubic.
   * @param is_even_odd Whether the fill rule is even-odd or non-zero.
   * @param is_culling Whether the path is culled.
   * @param z_index The z-index of the span.
   * @param transform_index The index of the transform to apply to the span.
   */
  PathInstance(
    const vec2 position,
    const vec2 size,
    const vec4& color,
    const size_t curves_start_index,
    const size_t bands_start_index,
    const uint8_t horizontal_bands,
    const bool is_quadratic,
    const bool is_even_odd,
    const bool is_culling,
    const uint32_t z_index,
    const uint32_t transform_index
  ) :
    position(position), size(size), color(color * 255.0f) {
    const uint32_t u_curves_x = (static_cast<uint32_t>(curves_start_index % GK_CURVES_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_curves_y = (static_cast<uint32_t>(curves_start_index / GK_CURVES_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_bands_x = (static_cast<uint32_t>(bands_start_index % GK_BANDS_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_bands_y = (static_cast<uint32_t>(bands_start_index / GK_BANDS_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_bands_h = (horizontal_bands < 1) ? 0 : static_cast<uint32_t>(horizontal_bands - 1);
    const uint32_t u_is_quad = static_cast<uint32_t>(is_quadratic);
    const uint32_t u_is_eodd = static_cast<uint32_t>(is_even_odd);
    const uint32_t u_is_cull = static_cast<uint32_t>(is_culling);
    const uint32_t u_transform_index = (transform_index << 20) >> 20;

    attr_1 = (u_is_cull << 26) | (u_is_quad << 25) | (u_is_eodd << 24) | (u_curves_x << 12) | (u_curves_y);
    attr_2 = (u_bands_h << 24) | (u_bands_x << 12) | (u_bands_y);
    attr_3 = (z_index << 12) | (u_transform_index);
  }
};

/**
 * @brief Represents an intersection point.
 *
 * @struct Intersection
 */
struct Intersection {
  float x;        /* The x-coordinate of the intersection. */
  bool downwards; /* Whether the intersection is downwards, used for non-zero fill rule. */
};

/**
 * @brief A range represinting a span.
 *
 * @struct Span
 */
struct Span {
  float min; /* The minimum x-coordinate of the span. */
  float max; /* The maximum x-coordinate of the span. */
};

/**
 * @brief Represents a band containing the culling data of a path.
 *
 * @struct Band
 */
struct Band {
  float top_y;                      /* The top y-coordinates of the band. */
  float bottom_y;                   /* The bottom y-coordinates of the band. */

  std::vector<Span> disabled_spans; /* The filled spans of the band. */
  std::vector<Span> filled_spans;   /* The filled spans of the band. */

  /**
   * @brief Disables the spans intersecting with the given curve.
   *
   * @param min The minimum x-coordinate of the curve.
   * @param max The maximum x-coordinate of the curve.
   */
  void push_curve(float min, float max) {
    if (min == max) {
      min -= math::geometric_epsilon<float>;
      max += math::geometric_epsilon<float>;
    }

    if (disabled_spans.empty()) {
      disabled_spans.emplace_back(Span{min, max});
      return;
    }

    int unioned = 0;
    int potential_index = disabled_spans.size();

    for (int i = 0; i < disabled_spans.size(); i++) {
      if (min >= disabled_spans[i].min && max <= disabled_spans[i].max) {
        /* The new span is completely within a larger span. */
        return;
      }

      const vec2 intersection = {std::max(min, disabled_spans[i].min), std::min(max, disabled_spans[i].max)};

      if (intersection.x <= intersection.y) {
        /* If there is an intersection, we can perform an union. */

        disabled_spans[i].min = std::min(disabled_spans[i].min, min);
        disabled_spans[i].max = std::max(disabled_spans[i].max, max);

        unioned++;
      } else if (min < disabled_spans[i].min) {
        potential_index = std::min(potential_index, i);
      }
    }

    if (unioned == 0) {
      /* If there was no union, we can insert the new span at the potensial index. */
      disabled_spans.insert(disabled_spans.begin() + potential_index, Span{min, max});
      return;
    }

    /* We can now remove the spans that are completely within the new span. */

    for (Span& span1 : disabled_spans) {
      if (span1.min == span1.max) continue;

      for (Span& span2 : disabled_spans) {
        if (&span1 == &span2 || span2.min == span2.max) {
          continue;
        }

        const vec2 intersection = {std::max(span1.min, span2.min), std::min(span1.max, span2.max)};

        if (intersection.x <= intersection.y) {
          /* If there is an intersection, we can perform an union and disable the second span. */

          span1.min = std::min(span1.min, span2.min);
          span1.max = std::max(span1.max, span2.max);

          span2.min = span2.max = 0;
        }
      }
    }

    /* We can erase all the spans that have zero width. */

    disabled_spans.erase(
      std::remove_if(disabled_spans.begin(), disabled_spans.end(), [](const Span& span) { return span.min == span.max; }),
      disabled_spans.end()
    );
  }
};

/**
 * @brief Represents a filled span instance (24 bytes).
 *
 * @struct FilledSpanInstance
 */
struct FilledSpanInstance {
  vec2 position;   /* | position.x (32) | position.y (32) | */
  vec2 size;       /* | size.x (32) | size.y (32) | */
  uvec4 color;     /* | color.rgba (32) | */
  uint32_t attr_1; /* | z_index (20) - transform_index (12) | */

  /**
   * @brief Constructs a new RectInstance object.
   *
   * @param position The position of the span.
   * @param size The size of the span.
   * @param color The color of the span.
   * @param z_index The z-index of the span.
   * @param transform_index The index of the transform to apply to the span.
   */
  FilledSpanInstance(
    const vec2 position,
    const vec2 size,
    const vec4& color,
    const uint32_t z_index,
    const uint32_t transform_index
  ) :
    position(position), size(size), color(color * 255.0f) {
    const uint32_t u_transform_index = (transform_index << 20) >> 20;

    attr_1 = (z_index << 12) | (u_transform_index);
  }
};

/**
 * @brief Represents a boundary span instance (32 bytes).
 *
 * @struct BoundarySpanInstance
 * @note There are 6 bits of padding left in the struct.
 */
struct BoundarySpanInstance {
  vec2 position;   /* | position.x (32) | position.y (32) | */
  vec2 size;       /* | size.x (32) | size.y (32) |*/
  uvec4 color;     /* | color.rgba (32) | */
  uint32_t attr_1; /* | winding (16) - curves_count (16) | */
  uint32_t attr_2; /* | (6) - is_quad (1) - is_eodd (1) - start_x (12) - start_y (12) | */
  uint32_t attr_3; /* | z_index (20) - transform_index (12) | */

  /**
   * @brief Constructs a new BoundarySpanInstance object.
   *
   * @param position The position of the top-left corner of the span.
   * @param size The size of the span, can be negative.
   * @param color The color of the span.
   * @param winding The left side winding to start from.
   * @param curves_start_index The index of the first curve in the curves texture.
   * @param curves_count The number of curves in the span.
   * @param is_quadratic Whether the curves are quadratic or cubic.
   * @param is_even_odd Whether the fill rule is even-odd or non-zero.
   * @param z_index The z-index of the span.
   * @param transform_index The index of the transform to apply to the span.
   */
  BoundarySpanInstance(
    const vec2 position,
    const vec2 size,
    const vec4& color,
    const int16_t winding,
    const size_t curves_start_index,
    const uint16_t curves_count,
    const bool is_quadratic,
    const bool is_even_odd,
    const uint32_t z_index,
    const uint32_t transform_index
  ) :
    position(position), size(size), color(color * 255.0f) {
    const uint32_t u_winding = static_cast<uint32_t>(static_cast<int32_t>(winding) + 32768);
    const uint32_t u_curves_count = static_cast<uint32_t>(curves_count);
    const uint32_t u_is_quad = static_cast<uint32_t>(is_quadratic);
    const uint32_t u_is_eodd = static_cast<uint32_t>(is_even_odd);
    const uint32_t u_start_x = (static_cast<uint32_t>(curves_start_index % GK_CURVES_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_start_y = (static_cast<uint32_t>(curves_start_index / GK_CURVES_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_transform_index = (transform_index << 20) >> 20;

    attr_1 = (u_winding << 16) | (u_curves_count);
    attr_2 = (u_is_quad << 25) | (u_is_eodd << 24) | (u_start_x << 12) | (u_start_y);
    attr_3 = (z_index << 12) | (u_transform_index);
  }
};

/**
 * @brief Represents a rect to be rendered using instancing.
 *
 * @struct RectInstance
 */
struct RectInstance {
  vec2 position; /* position.xy */
  vec2 size;     /* size.xy */
  uvec4 color;   /* color.rgba */

  /**
   * @brief Constructs a new RectInstance object.
   *
   * @param position The position of the rect.
   * @param size The size of the rect.
   * @param color The color of the rect.
   */
  RectInstance(const vec2 position, const vec2 size, const vec4& color) :
    position(position), size(size), color(color * 255.0f) { }
};

/**
 * @brief Represents a circle to be rendered using instancing.
 */
struct CircleInstance {
  vec2 position; /* position.xy */
  float radius;  /* radius */
  uvec4 color;   /* color.rgba */

  /**
   * @brief Constructs a new CircleInstance object.
   *
   * @param position The position of the circle.
   * @param radius The radius of the circle.
   * @param color The color of the circle.
   */
  CircleInstance(const vec2 position, const float radius, const vec4& color) :
    position(position), radius(radius), color(color * 255.0f) { }
};

/**
 * @brief Represents an image to be rendered using instancing.
 *
 * @struct ImageInstance
 */
struct ImageInstance {
  vec2 position; /* position.xy */
  vec2 size;     /* size.xy */

  /**
   * @brief Constructs a new ImageInstance object.
   *
   * @param position The position of the rect.
   * @param size The size of the rect.
   * @param color The color of the rect.
   */
  ImageInstance(const vec2 position, const vec2 size) : position(position), size(size) { }
};

/**
 * @brief Represents a line to be rendered using instancing.
 *
 * @struct LineInstance
 */
struct LineInstance {
  vec2 start;  /* start.xy */
  vec2 end;    /* end.xy */
  float width; /* width */
  uvec4 color; /* color.rgba */

  /**
   * @brief Constructs a new LineInstance object.
   *
   * @param start The start of the line.
   * @param end The end of the line.
   * @param width The width of the line.
   * @param color The color of the line.
   */
  LineInstance(const vec2 start, const vec2 end, const float width, const vec4& color) :
    start(start), end(end), width(width), color(color * 255.0f) { }
};

/**
 * @brief Represents a buffer of instances.
 *
 * @struct InstanceBuffer
 */
template <typename T>
struct InstanceBuffer {
  std::vector<std::vector<T>> batches; /* The instances. */

  uint32_t max_instances_per_batch;    /* The maximum number of instances for each batch. */

  /**
   * @brief Constructs a new InstanceBuffer object.
   *
   * @param max_instances_per_batch The maximum number of instances for each batch.
   */
  InstanceBuffer(const uint32_t max_instances_per_batch) : max_instances_per_batch(max_instances_per_batch) {
    batches.resize(1);
    batches[0].reserve(max_instances_per_batch);
  }

  /**
   * @brief Clears the instance batches.
   */
  inline void clear() {
    batches.resize(1);
    batches[0].clear();
  }

  /**
   * @brief Adds a new instance to the buffer.
   *
   * @param instance The instance to add.
   */
  inline void push_back(T&& instance) {
    if (batches.back().size() >= max_instances_per_batch) {
      batches.push_back({});
      batches.back().reserve(max_instances_per_batch);
    }

    batches.back().push_back(std::move(instance));
  }
};

/**
 * @brief Represents a mesh to be rendered using instancing.
 *
 * @struct InstancedData
 */
template <typename T>
struct InstancedData {
  InstanceBuffer<T> instances; /* The per-instance data. */

  GPU::Primitive primitive;    /* The primitive type of the mesh. */

  GPU::Buffer instance_buffer; /* The GPU instance buffer. */
  GPU::Buffer vertex_buffer;   /* The GPU vertex buffer. */

  size_t vertex_size;          /* The size of a vertex in bytes. */

  /**
   * @brief Initializes the instance data.
   *
   * @param buffer_size The maximum buffer size in bytes.
   * @param primitive The primitive type of the mesh.
   */
  InstancedData(
    const size_t buffer_size,
    const std::vector<vec2>& vertices,
    const GPU::Primitive primitive = GPU::Primitive::Triangles
  ) :
    primitive(primitive),
    instances(static_cast<uint32_t>(buffer_size / sizeof(T))),
    instance_buffer(GPU::BufferTarget::Vertex, GPU::BufferUploadMode::Dynamic, buffer_size),
    vertex_buffer(GPU::BufferTarget::Vertex, GPU::BufferUploadMode::Static, vertices.size() * sizeof(vec2), vertices.data()),
    vertex_size(sizeof(vec2)) { }

  /**
   * @brief Initializes the instance data.
   *
   * @param buffer_size The maximum buffer size in bytes.
   * @param primitive The primitive type of the mesh.
   */
  InstancedData(
    const size_t buffer_size,
    const std::vector<uvec2>& vertices,
    const GPU::Primitive primitive = GPU::Primitive::Triangles
  ) :
    primitive(primitive),
    instances(static_cast<uint32_t>(buffer_size / sizeof(T))),
    instance_buffer(GPU::BufferTarget::Vertex, GPU::BufferUploadMode::Dynamic, buffer_size),
    vertex_buffer(GPU::BufferTarget::Vertex, GPU::BufferUploadMode::Static, vertices.size() * sizeof(uvec2), vertices.data()),
    vertex_size(sizeof(uvec2)) { }

  InstancedData(const InstancedData&) = delete;
  InstancedData(InstancedData&&) = delete;

  InstancedData& operator=(const InstancedData&) = delete;
  InstancedData& operator=(InstancedData&&) = delete;

  /**
   * @brief Gets the maximum number of instances for each batch.
   *
   * @return The maximum number of instances for each batch.
   */
  inline uint32_t max_instances() const { return instances.max_instances_per_batch; }

  /**
   * @brief Clears the instance data.
   */
  virtual inline void clear() { instances.clear(); }
};

/**
 * @brief Represents the data of the path instances to render.
 *
 * @struct PathInstancedData
 */
struct PathInstancedData : public InstancedData<PathInstance> {
  std::vector<vec2> curves;    /* The control points of the curves. */
  std::vector<uint16_t> bands; /* The bands of the mesh. */

  GPU::Texture curves_texture;
  GPU::Texture bands_texture;

  /**
   * @brief Constructs a new PathInstanceData object.
   *
   * @param buffer_size The maximum buffer size.
   */
  PathInstancedData(const size_t buffer_size) :
    InstancedData<PathInstance>(buffer_size, quad_vertices({0, 0}, {1, 1})),
    curves_texture(
      GPU::TextureFormat::RGBA32F,
      {512, 512},
      GPU::TextureSamplingFlagNearestMin | GPU::TextureSamplingFlagNearestMag
    ),
    bands_texture(
      GPU::TextureFormat::R16UI,
      {512, 512},
      GPU::TextureSamplingFlagNearestMin | GPU::TextureSamplingFlagNearestMag
    ) {
    curves.reserve(512 * 512 * 2);
    bands.reserve(512 * 512);
  }

  /**
   * @brief Clears the instance data.
   */
  virtual inline void clear() override {
    InstancedData<PathInstance>::clear();

    curves.clear();
    bands.clear();

    curves.reserve(512 * 512 * 2);
    bands.reserve(512 * 512);
  }
};

/**
 * @brief Represents the data of the boundary span instances to render.
 *
 * @struct BoundarySpanInstancedData
 */
struct BoundarySpanInstancedData : public InstancedData<BoundarySpanInstance> {
  std::vector<vec2> curves;    /* The control points of the curves. */

  GPU::Texture curves_texture; /* The curves texture. */

  /**
   * @brief Constructs a new BoundarySpanInstancedData object.
   *
   * @param buffer_size The maximum buffer size.
   */
  BoundarySpanInstancedData(const size_t buffer_size) :
    InstancedData<BoundarySpanInstance>(buffer_size, quad_vertices({0, 0}, {1, 1})),
    curves_texture(
      GPU::TextureFormat::RGBA32F,
      {512, 512},
      GPU::TextureSamplingFlagNearestMin | GPU::TextureSamplingFlagNearestMag
    ) {
    curves.reserve(512 * 512 * 2);
  }

  /**
   * @brief Clears the instance data.
   */
  virtual inline void clear() override {
    InstancedData<BoundarySpanInstance>::clear();

    curves.clear();

    curves.reserve(512 * 512 * 2);
  }
};

/**
 * @brief Collects all the UI related options.
 *
 * @struct UIOptions
 */
struct UIOptions {
  vec2 vertex_size;       /* The size of a vertex. */
  vec2 vertex_inner_size; /* The size of the white part of a vertex. */

  float handle_radius;    /* The radius of an handle. */
  float line_width;       /* The default width of the lines. */

  vec4 primary_color;     /* The primary color of the UI. */
  vec4 primary_color_05;  /* The primary color 5% darker. */
};

}
