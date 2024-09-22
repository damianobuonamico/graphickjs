/**
 * @file renderer_data.h
 * @brief The file contains data structures used by the renderer.
 *
 * @todo maybe replace vectors with preallocated arrays
 * @todo double buffer the tile data
 * @todo batches of tile data overflow handling
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
 * @brief Represents a vertex used by the tile shader (32 bytes).
 *
 *
 * @note In attr 1, 7 bits for paint type are probably too much.
 * @note In attr_2, there are a few wasted bits (max bands is 32, but 256 is used here, bands coords are 12 bits instead of 10).
 * @note In attr_3, paint_coord is 10 bits instead of 8
 *
 * @struct Vertex
 */
struct TileVertex {
  vec2 position;                // | position.x (32) | position.y (32) |
  uvec4 color;                  // | color.rgba (32) |
  uint32_t tex_coord;           // | tex_coord.x (16) - tex_coord.y (16) |
  uint32_t tex_coord_curves;    // | tex_coord_curves.x (16) - tex_coord_curves.y (16) |
  uint32_t attr_1;              // | blend (5) - paint_type (7) - curves_x (10) - curves_y (10) |
  uint32_t attr_2;              // | bands_h (8) - bands_x (12) - bands_y (12) |
  uint32_t attr_3;              // | z_index (20) - is_quad (1) - is_eodd (1) - paint_coord (10) |

  /**
   * @brief Default constructor.
   */
  TileVertex() = default;

  /**
   * @brief Constructs a new TileVertex object.
   *
   * @param position The position of the vertex.
   * @param color The color of the vertex.
   * @param tex_coord The texture coordinate used for painting, should be in the range [0, 1].
   * @param tex_coord_curves The texture coordinate used for rasterization, should be in the range [0, 1].
   * @param attr_1 The attributes of the vertex, should be created using TileVertex::create_attr_1()
   * @param attr_2 The attributes of the vertex, should be created using TileVertex::create_attr_2()
   * @param attr_3 The attributes of the vertex, should be created using TileVertex::create_attr_3()
   */
  TileVertex(
    const vec2 position,
    const uvec4 color,
    const vec2 tex_coord,
    const vec2 tex_coord_curves,
    const uint32_t attr_1,
    const uint32_t attr_2,
    const uint32_t attr_3
  ) :
    position(position), color(color), attr_1(attr_1), attr_2(attr_2), attr_3(attr_3) {
    const uint32_t u_tex_coord_x = static_cast<uint32_t>(tex_coord.x * std::numeric_limits<uint16_t>::max());
    const uint32_t u_tex_coord_y = static_cast<uint32_t>(tex_coord.y * std::numeric_limits<uint16_t>::max());
    const uint32_t u_tex_coord_curves_x = static_cast<uint32_t>(tex_coord_curves.x * std::numeric_limits<uint16_t>::max());
    const uint32_t u_tex_coord_curves_y = static_cast<uint32_t>(tex_coord_curves.y * std::numeric_limits<uint16_t>::max());

    this->tex_coord = (u_tex_coord_x << 16) | u_tex_coord_y;
    this->tex_coord_curves = (u_tex_coord_curves_x << 16) | u_tex_coord_curves_y;
  }

  /**
   * @brief Creates the attr_1 attribute of the vertex.
   *
   * This method should be called once per object.
   *
   * @param blending_mode The blend mode of the vertex.
   * @param paint_type The paint type of the vertex.
   * @param curves_start_index The index of the first curve in the curves texture.
   * @return The attr_1 attribute.
   */
  static uint32_t create_attr_1(const uint8_t blending_mode, const uint8_t paint_type, const size_t curves_start_index) {
    const uint32_t u_curves_x = (static_cast<uint32_t>(curves_start_index % GK_CURVES_TEXTURE_SIZE) << 22) >> 22;
    const uint32_t u_curves_y = (static_cast<uint32_t>(curves_start_index / GK_CURVES_TEXTURE_SIZE) << 22) >> 22;
    const uint32_t u_paint_type = (static_cast<uint32_t>(paint_type) << 25) >> 25;
    const uint32_t u_blend = static_cast<uint32_t>(blending_mode);

    return (u_blend << 27) | (u_paint_type << 20) | (u_curves_x << 10) | (u_curves_y);
  }

  /**
   * @brief Creates the attr_2 attribute of the vertex.
   *
   * This method should be called once per object.
   *
   * @param horizontal_bands The number of horizontal bands.
   * @param bands_start_index The index of the first band in the bands texture.
   * @return The attr_2 attribute.
   */
  static uint32_t create_attr_2(const uint8_t horizontal_bands, const size_t bands_start_index) {
    const uint32_t u_bands_x = (static_cast<uint32_t>(bands_start_index % GK_BANDS_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_bands_y = (static_cast<uint32_t>(bands_start_index / GK_BANDS_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_bands_h = (horizontal_bands < 1) ? 0 : static_cast<uint32_t>(horizontal_bands - 1);

    return (u_bands_h << 24) | (u_bands_x << 12) | (u_bands_y);
  }

  /**
   * @brief Creates the attr_3 attribute of the vertex.
   *
   * This method should be called once per object.
   *
   * @param z_index The z-index of the vertex.
   * @param is_quadratic Whether the curves are quadratic or cubic.
   * @param is_even_odd Whether the fill rule is even-odd or non-zero.
   * @param paint_coord The paint coordinate of the vertex.
   * @return The attr_3 attribute.
   */
  static uint32_t create_attr_3(
    const uint32_t z_index,
    const bool is_quadratic,
    const bool is_even_odd,
    const uint16_t paint_coord
  ) {
    const uint32_t u_is_quad = static_cast<uint32_t>(is_quadratic);
    const uint32_t u_is_eodd = static_cast<uint32_t>(is_even_odd);
    const uint32_t u_paint_coord = (static_cast<uint32_t>(paint_coord) << 22) >> 22;

    return (z_index << 12) | (u_is_quad << 11) | (u_is_eodd << 10) | u_paint_coord;
  }
};

/**
 * @brief Represents a mask instance (32 bytes).
 *
 * @note There are 6 bits of padding left in the struct.
 * @note The maximum number of bands is currently 256 here and 32 in the renderer, so a few bits are wasted.
 *
 * @struct MaskInstance
 */
struct MaskInstance {
  vec2 position;      // | position.x (32) | position.y (32) |
  vec2 size;          // | size.x (32) | size.y (32) |
  vec2 tex_coords;    // | tex_coords.x (32) | tex_coords.y (32) |
  uint32_t attr_1;    // | (6) - is_quad (1) - is_eodd (1) - curves_x (12) - curves_y (12) |
  uint32_t attr_2;    // | bands_h (8) - bands_x (12) - bands_y (12) |

  MaskInstance(
    const vec2 position,
    const vec2 size,
    const vec2 tex_coords,
    const size_t curves_start_index,
    const size_t bands_start_index,
    const uint8_t horizontal_bands,
    const bool is_quadratic,
    const bool is_even_odd
  ) :
    position(position), size(size), tex_coords(tex_coords) {
    const uint32_t u_curves_x = (static_cast<uint32_t>(curves_start_index % GK_CURVES_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_curves_y = (static_cast<uint32_t>(curves_start_index / GK_CURVES_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_bands_x = (static_cast<uint32_t>(bands_start_index % GK_BANDS_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_bands_y = (static_cast<uint32_t>(bands_start_index / GK_BANDS_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_bands_h = (horizontal_bands < 1) ? 0 : static_cast<uint32_t>(horizontal_bands - 1);
    const uint32_t u_is_quad = static_cast<uint32_t>(is_quadratic);
    const uint32_t u_is_eodd = static_cast<uint32_t>(is_even_odd);

    attr_1 = (u_is_quad << 25) | (u_is_eodd << 24) | (u_curves_x << 12) | (u_curves_y);
    attr_2 = (u_bands_h << 24) | (u_bands_x << 12) | (u_bands_y);
  }
};

/**
 * @brief Represents a path instance (32 bytes), the main building block of the renderer.
 *
 * @note There are 5 bits of padding left in the struct.
 * @note The maximum number of bands is currently 256 here and 32 in the renderer, so a few bits are wasted.
 *
 * @struct PathInstance
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
 * @brief Represents the data of the mask instances to render.
 *
 * @struct MaskInstancedData
 */
struct MaskInstancedData : public InstancedData<MaskInstance> {
  std::vector<vec2> curves;    /* The control points of the curves. */
  std::vector<uint16_t> bands; /* The bands of the mesh. */

  GPU::Texture curves_texture;
  GPU::Texture bands_texture;

  /**
   * @brief Constructs a new MaskInstanceData object.
   *
   * @param buffer_size The maximum buffer size.
   */
  MaskInstancedData(const size_t buffer_size) :
    InstancedData<MaskInstance>(buffer_size, quad_vertices({0, 0}, {1, 1})),
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
    InstancedData<MaskInstance>::clear();

    curves.clear();
    bands.clear();

    curves.reserve(512 * 512 * 2);
    bands.reserve(512 * 512);
  }
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
 * @brief Represents the data of the tiles to render.
 *
 * @struct FilledSpanInstancedData
 * @todo replace count with size and add real count
 */
struct TileBatchedData {
  size_t vertices_count;             // The max number of vertices in the batch.
  size_t indices_count;              // The max number of indices in the batch.
  size_t curves_count;               // The max number of control points in the curves texture.
  size_t bands_count;                // The max number of indices in the bands texture.
  size_t gradient_count;             // The max number of pixels in the gradient texture.

  TileVertex* vertices;              // The vertices of the batch.
  TileVertex* vertices_ptr;          // The current index of the vertices.

  uint16_t* indices;                 // The indices of the batch, these are all static quads.
  uint16_t* indices_ptr;             // The current index of the indices.

  vec2* curves;                      // The control points of the curves.
  vec2* curves_ptr;                  // The current index of the curves.

  uint16_t* bands;                   // The bands of the meshes.
  uint16_t* bands_ptr;               // The current index of the bands.

  uvec4* gradients;                  // The gradients of the meshes.
  uvec4* gradients_ptr;              // The current index of the gradients.

  GPU::Texture curves_texture;       // The curves texture.
  GPU::Texture bands_texture;        // The bands texture.
  GPU::Texture gradients_texture;    // The gradients texture.

  GPU::Buffer vertex_buffer;         // The GPU vertex buffer.
  GPU::Buffer index_buffer;          // The GPU index buffer.

  GPU::Primitive primitive;          // The primitive type of the mesh.

  /**
   * @brief Constructs a new TileBatchedData object.
   *
   * @param buffer_size The maximum buffer size in bytes.
   */
  TileBatchedData(const size_t buffer_size) :
    primitive(GPU::Primitive::Triangles),
    vertices_count(buffer_size / sizeof(TileVertex)),
    indices_count(vertices_count * 3 / 2),
    curves_count(GK_CURVES_TEXTURE_SIZE * GK_CURVES_TEXTURE_SIZE * 2),
    bands_count(GK_BANDS_TEXTURE_SIZE * GK_BANDS_TEXTURE_SIZE),
    gradient_count(GK_GRADIENTS_TEXTURE_WIDTH * GK_GRADIENTS_TEXTURE_HEIGHT),
    curves_texture(
      GPU::TextureFormat::RGBA32F,
      ivec2{GK_CURVES_TEXTURE_SIZE},
      GPU::TextureSamplingFlagNearestMin | GPU::TextureSamplingFlagNearestMag
    ),
    bands_texture(
      GPU::TextureFormat::R16UI,
      ivec2{GK_BANDS_TEXTURE_SIZE},
      GPU::TextureSamplingFlagNearestMin | GPU::TextureSamplingFlagNearestMag
    ),
    gradients_texture(
      GPU::TextureFormat::RGBA8,
      ivec2{GK_GRADIENTS_TEXTURE_WIDTH, GK_GRADIENTS_TEXTURE_HEIGHT},
      GPU::TextureSamplingFlagNone
    ),
    vertex_buffer(GPU::BufferTarget::Vertex, GPU::BufferUploadMode::Dynamic, vertices_count * sizeof(TileVertex)),
    index_buffer(GPU::BufferTarget::Index, GPU::BufferUploadMode::Static, indices_count * sizeof(uint16_t)) {

    vertices = new TileVertex[vertices_count];
    indices = new uint16_t[indices_count];
    curves = new vec2[curves_count];
    bands = new uint16_t[bands_count];
    gradients = new uvec4[gradient_count];

    vertices_ptr = vertices;
    indices_ptr = indices;
    curves_ptr = curves;
    bands_ptr = bands;
    gradients_ptr = gradients;

    // Fill the index buffer with static quads.
    for (size_t i = 0; i < indices_count; i += 6) {
      indices[i + 0] = i / 6 * 4 + 0;
      indices[i + 1] = i / 6 * 4 + 1;
      indices[i + 2] = i / 6 * 4 + 2;
      indices[i + 3] = i / 6 * 4 + 2;
      indices[i + 4] = i / 6 * 4 + 3;
      indices[i + 5] = i / 6 * 4 + 0;
    }

    index_buffer.upload(indices, indices_count * sizeof(uint16_t));

    // Fill the gradients texture with white.
    for (size_t i = 0; i < gradient_count; i++) {
      gradients[i] = uvec4(i % GK_GRADIENTS_TEXTURE_HEIGHT);
    }

    gradients_texture.upload(gradients, gradient_count * sizeof(uvec4));
  }

  /**
   * @brief Destroys the TileBatchedData object.
   */
  ~TileBatchedData() {
    delete[] vertices;
    delete[] indices;
    delete[] curves;
    delete[] bands;
    delete[] gradients;
  }

  /**
   * @brief Clears the batched data.
   */
  inline void clear() {
    vertices_ptr = vertices;
    indices_ptr = indices;
    curves_ptr = curves;
    bands_ptr = bands;
    gradients_ptr = gradients;
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
