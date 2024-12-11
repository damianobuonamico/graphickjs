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

#include "drawable.h"
#include "properties.h"
#include "renderer_settings.h"

#include "../math/mat2x3.h"
#include "../math/rect.h"
#include "../math/scalar.h"
#include "../math/vec4.h"

#include "../utils/defines.h"
#include "../utils/half.h"
#include "../utils/uuid.h"

namespace graphick::editor {
class Cache;
}

namespace graphick::renderer {

/**
 * @brief Represents the viewport of the renderer.
 *
 * The viewport is the area of the screen where the renderer will draw.
 */
struct Viewport {
  ivec2 size;       // The size of the viewport.
  dvec2 position;   // The position of the viewport.

  double zoom;      // The zoom level of the viewport (it is pre-multiplied by the dpr).
  double dpr;       // The device pixel ratio.

  vec4 background;  // The background color to clear the viewport with.

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
  Viewport(const ivec2 size,
           const dvec2 position,
           const double zoom,
           const double dpr,
           const vec4& background)
      : size(size),
        position(position),
        zoom(zoom),
        dpr(dpr),
        background(background),
        m_visible({-position, dvec2(size) / zoom - position})
  {
  }

  /**
   * @brief Returns the scene-space visible area.
   *
   * @return The scene-space rectangle that is visible in the viewport.
   */
  inline drect visible() const
  {
    return m_visible;
  }

  /**
   * @brief Converts a point from client-space to scene-space.
   *
   * @param p The point in client-space.
   * @return The point in scene-space.
   */
  inline dvec2 project(const dvec2 p) const
  {
    return p / zoom - position;
  }

 private:
  drect m_visible;  // The visible area of the viewport in scene-space coordinates.
};

/**
 * @brief Represents the options used to render the scene.
 */
struct RenderOptions {
  Viewport viewport;     // The viewport to render to.
  editor::Cache* cache;  // The cache to use for rendering, can be nullptr.

  bool ignore_cache;     // Whether to ignore the cache and redraw everything.
};

/**
 * @brief Generates the vertices of a quad.
 *
 * @param min The minimum point of the quad.
 * @param max The maximum point of the quad.
 * @return The vertices of the quad.
 */
inline std::vector<uvec2> quad_vertices(const uvec2 min, const uvec2 max)
{
  return {min, uvec2{max.x, min.y}, max, max, uvec2{min.x, max.y}, min};
}

/**
 * @brief Represents an intersection point.
 */
struct Intersection {
  double x;        // The x-coordinate of the intersection.
  bool downwards;  // Whether the intersection is downwards, used for non-zero fill rule.
};

/**
 * @brief A range represinting a span.
 */
struct Span {
  double min;  // The minimum x-coordinate of the span.
  double max;  // The maximum x-coordinate of the span.
};

/**
 * @brief Represents a band containing the culling data of a path.
 */
struct Band {
  std::vector<Span> disabled_spans;  // The filled spans of the band.
  std::vector<Span> filled_spans;    // The filled spans of the band.

  /**
   * @brief Disables the spans intersecting with the given curve.
   *
   * @param min The minimum x-coordinate of the curve.
   * @param max The maximum x-coordinate of the curve.
   */
  void push_curve(double min, double max)
  {
    if (min == max) {
      min -= math::geometric_epsilon<double>;
      max += math::geometric_epsilon<double>;
    }

    if (disabled_spans.empty()) {
      disabled_spans.emplace_back(Span{min, max});
      return;
    }

    int unioned = 0;
    int potential_index = disabled_spans.size();

    for (int i = 0; i < disabled_spans.size(); i++) {
      if (min >= disabled_spans[i].min && max <= disabled_spans[i].max) {
        // The new span is completely within a larger span.
        return;
      }

      const dvec2 intersection = {std::max(min, disabled_spans[i].min),
                                  std::min(max, disabled_spans[i].max)};

      if (intersection.x <= intersection.y) {
        // If there is an intersection, we can perform an union.

        disabled_spans[i].min = std::min(disabled_spans[i].min, min);
        disabled_spans[i].max = std::max(disabled_spans[i].max, max);

        unioned++;
      } else if (min < disabled_spans[i].min) {
        potential_index = std::min(potential_index, i);
      }
    }

    if (unioned == 0) {
      // If there was no union, we can insert the new span at the potensial index.
      disabled_spans.insert(disabled_spans.begin() + potential_index, Span{min, max});
      return;
    }

    // We can now remove the spans that are completely within the new span.

    for (Span& span1 : disabled_spans) {
      if (span1.min == span1.max)
        continue;

      for (Span& span2 : disabled_spans) {
        if (&span1 == &span2 || span2.min == span2.max) {
          continue;
        }

        const dvec2 intersection = {std::max(span1.min, span2.min),
                                    std::min(span1.max, span2.max)};

        if (intersection.x <= intersection.y) {
          // If there is an intersection, we can perform an union and disable the second span.

          span1.min = std::min(span1.min, span2.min);
          span1.max = std::max(span1.max, span2.max);

          span2.min = span2.max = 0;
        }
      }
    }

    // We can erase all the spans that have zero width.

    disabled_spans.erase(std::remove_if(disabled_spans.begin(),
                                        disabled_spans.end(),
                                        [](const Span& span) { return span.min == span.max; }),
                         disabled_spans.end());
  }
};

/**
 * @brief Groups the data required to be populated in order to generate a drawable from a path.
 */
struct PathData {
  Drawable& drawable;                         // The drawable to populate.

  const std::array<vec2, 4>& texture_coords;  // The texture coordinates of the bounding box.

  const drect& bounding_rect;                 // The bounding rectangle of the path.
  const dvec2 bounds_size;                    // The size of the bounding rectangle.

  const Fill& fill;                           // The Fill properties to use.

  const size_t num;                           // The number of curves in the path.
  const size_t curves_offset;                 // The offset of the curves in the drawable.
  const size_t bands_offset;                  // The offset of the bands in the drawable.

  const bool culling;                         // Whether to perform culling.

  std::vector<dvec2> min;                     // The cached minimum points of the curves.
  std::vector<dvec2> max;                     // The cached maximum points of the curves.

  double band_delta;                          // The height of a band.
  uint8_t horizontal_bands;                   // The number of horizontal bands.
};

/**
 * @brief Represents a line to be rendered using instancing.
 */
struct LineInstance {
  vec2 start;   // start.xy */
  vec2 end;     // end.xy */
  float width;  // width */
  uvec4 color;  // color.rgba */

  /**
   * @brief Constructs a new LineInstance object.
   *
   * @param start The start of the line.
   * @param end The end of the line.
   * @param width The width of the line.
   * @param color The color of the line.
   */
  LineInstance(const vec2 start, const vec2 end, const float width, const vec4& color)
      : start(start), end(end), width(width), color(color * 255.0f)
  {
  }
};

/**
 * @brief Represents a rect to be rendered using instancing.
 */
struct RectInstance {
  vec2 position;  // position.xy */
  vec2 size;      // size.xy */
  uvec4 color;    // color.rgba */

  /**
   * @brief Constructs a new RectInstance object.
   *
   * @param position The position of the rect.
   * @param size The size of the rect.
   * @param color The color of the rect.
   */
  RectInstance(const vec2 position, const vec2 size, const vec4& color)
      : position(position), size(size), color(color * 255.0f)
  {
  }
};

/**
 * @brief Represents a circle to be rendered using instancing.
 */
struct CircleInstance {
  vec2 position;  // position.xy */
  float radius;   // radius */
  uvec4 color;    // color.rgba */

  /**
   * @brief Constructs a new CircleInstance object.
   *
   * @param position The position of the circle.
   * @param radius The radius of the circle.
   * @param color The color of the circle.
   */
  CircleInstance(const vec2 position, const float radius, const vec4& color)
      : position(position), radius(radius), color(color * 255.0f)
  {
  }
};

/**
 * @brief Represents an image to be rendered using instancing.
 */
struct ImageInstance {
  vec2 position;  // position.xy */
  vec2 size;      // size.xy */

  /**
   * @brief Constructs a new ImageInstance object.
   *
   * @param position The position of the rect.
   * @param size The size of the rect.
   * @param color The color of the rect.
   */
  ImageInstance(const vec2 position, const vec2 size) : position(position), size(size) {}
};

/**
 * @brief Represents a buffer of instances.
 */
template<typename T>
struct InstanceBuffer {
  std::vector<std::vector<T>> batches;  // The instances.

  uint32_t max_instances_per_batch;     // The maximum number of instances for each batch.

  /**
   * @brief Constructs a new InstanceBuffer object.
   *
   * @param max_instances_per_batch The maximum number of instances for each batch.
   */
  InstanceBuffer(const uint32_t max_instances_per_batch)
      : max_instances_per_batch(max_instances_per_batch)
  {
    batches.resize(1);
    batches[0].reserve(max_instances_per_batch);
  }

  /**
   * @brief Clears the instance batches.
   */
  inline void clear()
  {
    batches.resize(1);
    batches[0].clear();
  }

  /**
   * @brief Adds a new instance to the buffer.
   *
   * @param instance The instance to add.
   */
  inline void push_back(T&& instance)
  {
    if (batches.back().size() >= max_instances_per_batch) {
      batches.push_back({});
      batches.back().reserve(max_instances_per_batch);
    }

    batches.back().push_back(std::move(instance));
  }
};

/**
 * @brief Represents a mesh to be rendered using instancing.
 */
template<typename T>
struct InstancedData {
  InstanceBuffer<T> instances;  // The per-instance data.

  GPU::Primitive primitive;     // The primitive type of the mesh.

  GPU::Buffer instance_buffer;  // The GPU instance buffer.
  GPU::Buffer vertex_buffer;    // The GPU vertex buffer.

  size_t vertex_size;           // The size of a vertex in bytes.

  /**
   * @brief Initializes the instance data.
   *
   * @param buffer_size The maximum buffer size in bytes.
   * @param primitive The primitive type of the mesh.
   */
  InstancedData(const size_t buffer_size,
                const std::vector<vec2>& vertices,
                const GPU::Primitive primitive = GPU::Primitive::Triangles)
      : primitive(primitive),
        instances(static_cast<uint32_t>(buffer_size / sizeof(T))),
        instance_buffer(GPU::BufferTarget::Vertex, GPU::BufferUploadMode::Dynamic, buffer_size),
        vertex_buffer(GPU::BufferTarget::Vertex,
                      GPU::BufferUploadMode::Static,
                      vertices.size() * sizeof(vec2),
                      vertices.data()),
        vertex_size(sizeof(vec2))
  {
  }

  /**
   * @brief Initializes the instance data.
   *
   * @param buffer_size The maximum buffer size in bytes.
   * @param primitive The primitive type of the mesh.
   */
  InstancedData(const size_t buffer_size,
                const std::vector<uvec2>& vertices,
                const GPU::Primitive primitive = GPU::Primitive::Triangles)
      : primitive(primitive),
        instances(static_cast<uint32_t>(buffer_size / sizeof(T))),
        instance_buffer(GPU::BufferTarget::Vertex, GPU::BufferUploadMode::Dynamic, buffer_size),
        vertex_buffer(GPU::BufferTarget::Vertex,
                      GPU::BufferUploadMode::Static,
                      vertices.size() * sizeof(uvec2),
                      vertices.data()),
        vertex_size(sizeof(uvec2))
  {
  }

  InstancedData(const InstancedData&) = delete;
  InstancedData(InstancedData&&) = delete;

  InstancedData& operator=(const InstancedData&) = delete;
  InstancedData& operator=(InstancedData&&) = delete;

  /**
   * @brief Gets the maximum number of instances for each batch.
   *
   * @return The maximum number of instances for each batch.
   */
  inline uint32_t max_instances() const
  {
    return instances.max_instances_per_batch;
  }

  /**
   * @brief Clears the instance data.
   */
  virtual inline void clear()
  {
    instances.clear();
  }
};

/**
 * @brief Represents the data of a single batch.
 * @todo maybe use double buffering
 */
struct BatchData {
  size_t max_gradients;            // The maximum number of gradients in the batch.

  uvec4* gradients;                // The gradients of the meshes.
  uvec4* gradients_ptr;            // The current index of the gradients.

  GPU::Texture gradients_texture;  // The gradients texture.

  /**
   * @brief Constructs a new BatchData object.
   */
  BatchData()
      : max_gradients(GK_GRADIENTS_TEXTURE_HEIGHT),
        gradients_texture(GPU::TextureFormat::RGBA8,
                          ivec2{GK_GRADIENTS_TEXTURE_WIDTH, GK_GRADIENTS_TEXTURE_HEIGHT},
                          GPU::TextureSamplingFlagNone)
  {
    gradients = new uvec4[max_gradients * GK_GRADIENTS_TEXTURE_WIDTH];
    gradients_ptr = gradients;
  }

  /**
   * @brief Destroys the BatchData object.
   */
  ~BatchData()
  {
    delete[] gradients;
  }

  /**
   * @brief Gets the number of gradients currently in the batch.
   *
   * @return The number of gradients in the batch.
   */
  inline size_t gradients_count() const
  {
    return (gradients_ptr - gradients) / GK_GRADIENTS_TEXTURE_WIDTH;
  }

  /**
   * @brief Clears the batch data.
   */
  inline void clear()
  {
    gradients_ptr = gradients;
  }

  /**
   * @brief Checks if the batch can handle the given number of gradients.
   *
   * @param gradients The number of gradients to add.
   * @return Whether the batch can handle the gradients.
   */
  inline bool can_handle_gradients(const size_t gradients) const
  {
    return this->gradients_count() + gradients < max_gradients;
  }
};

/**
 * @brief Represents the data of a single tile batch.
 */
struct TileBatchData {
  size_t max_vertices;          // The maximum number of vertices in the batch.
  size_t max_indices;           // The maximum number of indices in the batch.
  size_t max_curves;            // The maximum number of control points in the curves texture.
  size_t max_bands;             // The maximum number of indices in the bands texture.

  TileVertex* vertices;         // The vertices of the batch.
  TileVertex* vertices_ptr;     // The current index of the vertices.

  uint16_t* indices;            // The indices of the batch, these are all static quads.
  uint16_t* indices_ptr;        // The current index of the indices.

  vec2* curves;                 // The control points of the curves.
  vec2* curves_ptr;             // The current index of the curves.

  uint16_t* bands;              // The bands of the meshes.
  uint16_t* bands_ptr;          // The current index of the bands.

  GPU::Buffer vertex_buffer;    // The GPU vertex buffer.
  GPU::Buffer index_buffer;     // The GPU index buffer.
  GPU::Texture curves_texture;  // The curves texture.
  GPU::Texture bands_texture;   // The bands texture.

  GPU::Primitive primitive;     // The primitive type of the mesh.

  /**
   * @brief Constructs a new TileBatchData object.
   */
  TileBatchData(const size_t buffer_size)
      : primitive(GPU::Primitive::Triangles),
        max_vertices(buffer_size / sizeof(TileVertex)),
        max_indices(max_vertices * 3 / 2),
        max_curves(GK_CURVES_TEXTURE_SIZE * GK_CURVES_TEXTURE_SIZE),
        max_bands(GK_BANDS_TEXTURE_SIZE * GK_BANDS_TEXTURE_SIZE),
        vertex_buffer(GPU::BufferTarget::Vertex,
                      GPU::BufferUploadMode::Dynamic,
                      max_vertices * sizeof(TileVertex)),
        index_buffer(GPU::BufferTarget::Index,
                     GPU::BufferUploadMode::Static,
                     max_indices * sizeof(uint16_t)),
        curves_texture(GPU::TextureFormat::RGBA32F,
                       ivec2{GK_CURVES_TEXTURE_SIZE},
                       GPU::TextureSamplingFlagNearestMin | GPU::TextureSamplingFlagNearestMag),
        bands_texture(GPU::TextureFormat::R16UI,
                      ivec2{GK_BANDS_TEXTURE_SIZE},
                      GPU::TextureSamplingFlagNearestMin | GPU::TextureSamplingFlagNearestMag)
  {
    vertices = new TileVertex[max_vertices];
    indices = new uint16_t[max_indices];
    curves = new vec2[max_curves * 2];
    bands = new uint16_t[max_bands];

    vertices_ptr = vertices;
    indices_ptr = indices;
    curves_ptr = curves;
    bands_ptr = bands;

    // Fill the index buffer with static quads.
    for (size_t i = 0; i < max_indices - 5; i += 6) {
      indices[i + 0] = i / 6 * 4 + 0;
      indices[i + 1] = i / 6 * 4 + 1;
      indices[i + 2] = i / 6 * 4 + 2;
      indices[i + 3] = i / 6 * 4 + 2;
      indices[i + 4] = i / 6 * 4 + 3;
      indices[i + 5] = i / 6 * 4 + 0;
    }

    index_buffer.upload(indices, max_indices * sizeof(uint16_t));
  }

  /**
   * @brief Destroys the TileBatchData object.
   */
  ~TileBatchData()
  {
    delete[] vertices;
    delete[] indices;
    delete[] curves;
    delete[] bands;
  }

  /**
   * @brief Gets the number of vertices currently in the batch.
   *
   * @return The number of vertices in the batch.
   */
  inline size_t vertices_count() const
  {
    return vertices_ptr - vertices;
  }

  /**
   * @brief Gets the number of indices currently in the batch.
   *
   * @return The number of indices in the batch.
   */
  inline size_t indices_count() const
  {
    return vertices_count() * 3 / 2;
  }

  /**
   * @brief Gets the number of curves currently in the batch.
   *
   * @return The number of curves in the batch.
   */
  inline size_t curves_count() const
  {
    return (curves_ptr - curves) / 2;
  }

  /**
   * @brief Gets the number of bands currently in the batch.
   *
   * @return The number of bands in the batch.
   */
  inline size_t bands_count() const
  {
    return bands_ptr - bands;
  }

  /**
   * @brief Clears the batch data.
   */
  inline void clear()
  {
    vertices_ptr = vertices;
    indices_ptr = indices;
    curves_ptr = curves;
    bands_ptr = bands;
  }

  /**
   * @brief Checks if the batch can handle the given number of curves.
   *
   * @param curves The number of curves to add.
   * @return Whether the batch can handle the curves.
   */
  inline bool can_handle_quads(const size_t quads = 1) const
  {
    return this->vertices_count() + quads * 4 < max_vertices;
  }

  /**
   * @brief Checks if the batch can handle the given number of curves.
   *
   * @param curves The number of curves to add.
   * @return Whether the batch can handle the curves.
   */
  inline bool can_handle_curves(const size_t curves) const
  {
    return this->curves_count() + curves < max_curves;
  }

  /**
   * @brief Checks if the batch can handle the given number of bands.
   *
   * @param bands The number of bands to add.
   * @return Whether the batch can handle the bands.
   */
  inline bool can_handle_bands(const size_t bands) const
  {
    return this->bands_count() + bands < max_bands;
  }

  /**
   * @brief Uploads the vertices, curves and bands to the buffers.
   *
   * @param drawable The drawable with the tile vertices, curves and bands to upload.
   * @param z_index The z-index of the drawable.
   */
  inline void upload(const Drawable& drawable, const uint32_t z_index)
  {
    memcpy(vertices_ptr, drawable.tiles.data(), drawable.tiles.size() * sizeof(TileVertex));
    memcpy(curves_ptr, drawable.curves.data(), drawable.curves.size() * sizeof(vec2));
    memcpy(bands_ptr, drawable.bands.data(), drawable.bands.size() * sizeof(uint16_t));

    const size_t curves_start_index = curves_count();
    const size_t bands_start_index = bands_count();

    const TileVertex* vertices_end_ptr = vertices_ptr + drawable.tiles.size();

    for (; vertices_ptr < vertices_end_ptr; vertices_ptr++) {
      vertices_ptr->add_offset_to_curves(curves_start_index);
      vertices_ptr->add_offset_to_bands(bands_start_index);
      vertices_ptr->update_z_index(z_index);
    }

    curves_ptr += drawable.curves.size();
    bands_ptr += drawable.bands.size();
  }

  /**
   * @brief Uploads the vertices, curves and bands to the buffers.
   *
   * @param drawable The drawable with the tile vertices, curves and bands to upload.
   * @param z_index The z-index of the drawable.
   * @param textures The texture bindings to use for the drawable.
   */
  inline void upload(const Drawable& drawable,
                     const uint32_t z_index,
                     const std::unordered_map<uuid, uint32_t>& textures)
  {
    memcpy(vertices_ptr, drawable.tiles.data(), drawable.tiles.size() * sizeof(TileVertex));
    memcpy(curves_ptr, drawable.curves.data(), drawable.curves.size() * sizeof(vec2));
    memcpy(bands_ptr, drawable.bands.data(), drawable.bands.size() * sizeof(uint16_t));

    const size_t curves_start_index = curves_count();
    const size_t bands_start_index = bands_count();

    size_t local_z_index = z_index;

    const TileVertex* vertices_start_ptr = vertices_ptr;

    for (const DrawablePaintBinding& binding : drawable.paints) {
      const TileVertex* vertices_end_ptr = vertices_start_ptr + binding.last_tile_index;

      if (binding.paint_type == Paint::Type::TexturePaint) {
        const auto it = textures.find(binding.paint_id);

        if (it == textures.end()) {
          continue;
        }

        const uint32_t texture_index = it->second;

        for (; vertices_ptr < vertices_end_ptr; vertices_ptr++) {
          vertices_ptr->add_offset_to_curves(curves_start_index);
          vertices_ptr->add_offset_to_bands(bands_start_index);
          vertices_ptr->update_z_index(local_z_index);
          vertices_ptr->update_paint_coord(texture_index);
        }
      } else {
        for (; vertices_ptr < vertices_end_ptr; vertices_ptr++) {
          vertices_ptr->add_offset_to_curves(curves_start_index);
          vertices_ptr->add_offset_to_bands(bands_start_index);
          vertices_ptr->update_z_index(local_z_index);
        }
      }

      local_z_index++;
    }

    curves_ptr += drawable.curves.size();
    bands_ptr += drawable.bands.size();
  }
};

/**
 * @brief Represents the data of a single fill batch.
 */
struct FillBatchData {
  size_t max_vertices;        // The maximum number of vertices in the batch.
  size_t max_indices;         // The maximum number of indices in the batch.

  FillVertex* vertices;       // The vertices of the batch.
  FillVertex* vertices_ptr;   // The current index of the vertices.

  uint16_t* indices;          // The indices of the batch, these are all static quads.
  uint16_t* indices_ptr;      // The current index of the indices.

  GPU::Buffer vertex_buffer;  // The GPU vertex buffer.
  GPU::Buffer index_buffer;   // The GPU index buffer.

  GPU::Primitive primitive;   // The primitive type of the mesh.

  /**
   * @brief Constructs a new FillBatchData object.
   *
   * @param buffer_size The maximum buffer size in bytes.
   */
  FillBatchData(const size_t buffer_size)
      : primitive(GPU::Primitive::Triangles),
        max_vertices(buffer_size / sizeof(FillVertex)),
        max_indices(max_vertices * 3 / 2),
        vertex_buffer(GPU::BufferTarget::Vertex,
                      GPU::BufferUploadMode::Dynamic,
                      max_vertices * sizeof(FillVertex)),
        index_buffer(GPU::BufferTarget::Index,
                     GPU::BufferUploadMode::Static,
                     max_indices * sizeof(uint16_t))
  {
    vertices = new FillVertex[max_vertices];
    indices = new uint16_t[max_indices];

    vertices_ptr = vertices;
    indices_ptr = indices;

    // Fill the index buffer with static quads.
    for (size_t i = 0; i < max_indices - 5; i += 6) {
      indices[i + 0] = i / 6 * 4 + 0;
      indices[i + 1] = i / 6 * 4 + 1;
      indices[i + 2] = i / 6 * 4 + 2;
      indices[i + 3] = i / 6 * 4 + 2;
      indices[i + 4] = i / 6 * 4 + 3;
      indices[i + 5] = i / 6 * 4 + 0;
    }

    index_buffer.upload(indices, max_indices * sizeof(uint16_t));
  }

  /**
   * @brief Destroys the FillBatchData object.
   */
  ~FillBatchData()
  {
    delete[] vertices;
    delete[] indices;
  }

  /**
   * @brief Gets the number of vertices currently in the batch.
   *
   * @return The number of vertices in the batch.
   */
  inline size_t vertices_count() const
  {
    return vertices_ptr - vertices;
  }

  /**
   * @brief Gets the number of indices currently in the batch.
   *
   * @return The number of indices in the batch.
   */
  inline size_t indices_count() const
  {
    return vertices_count() * 3 / 2;
  }

  /**
   * @brief Clears the batch data.
   */
  inline void clear()
  {
    vertices_ptr = vertices;
  }

  /**
   * @brief Checks if the batch can handle the given number of curves.
   *
   * @param curves The number of curves to add.
   * @return Whether the batch can handle the curves.
   */
  inline bool can_handle_quads(const size_t quads = 1) const
  {
    return this->vertices_count() + quads * 4 < max_vertices;
  }

  /**
   * @brief Uploads the vertices to the buffer.
   *
   * @param drawable The drawable with the fill vertices to upload.
   * @param z_index The z-index of the drawable.
   */
  inline void upload(const Drawable& drawable, const uint32_t z_index)
  {
    memcpy(vertices_ptr, drawable.fills.data(), drawable.fills.size() * sizeof(FillVertex));

    const FillVertex* vertices_end_ptr = vertices_ptr + drawable.fills.size();

    for (; vertices_ptr < vertices_end_ptr; vertices_ptr++) {
      vertices_ptr->update_z_index(z_index);
    }
  }

  /**
   * @brief Uploads the vertices, curves and bands to the buffers.
   *
   * @param drawable The drawable with the tile vertices, curves and bands to upload.
   * @param z_index The z-index of the drawable.
   * @param textures The texture bindings to use for the drawable.
   */
  inline void upload(const Drawable& drawable,
                     const uint32_t z_index,
                     const std::unordered_map<uuid, uint32_t>& textures)
  {
    memcpy(vertices_ptr, drawable.fills.data(), drawable.fills.size() * sizeof(FillVertex));

    uint32_t local_z_index = z_index;

    const FillVertex* vertices_start_ptr = vertices_ptr;

    for (const DrawablePaintBinding& binding : drawable.paints) {
      const FillVertex* vertices_end_ptr = vertices_start_ptr + binding.last_fill_index;

      if (binding.paint_type == Paint::Type::TexturePaint) {
        const auto it = textures.find(binding.paint_id);

        if (it == textures.end()) {
          continue;
        }

        const uint32_t texture_index = it->second;

        for (; vertices_ptr < vertices_end_ptr; vertices_ptr++) {
          vertices_ptr->update_z_index(local_z_index);
          vertices_ptr->update_paint_coord(texture_index);
        }
      } else {
        for (; vertices_ptr < vertices_end_ptr; vertices_ptr++) {
          vertices_ptr->update_z_index(local_z_index);
        }
      }

      local_z_index++;
    }
  }
};

struct Batch {
  TileBatchData tiles;
  FillBatchData fills;
  BatchData data;

  /**
   * @brief Constructs a new Batch object.
   */
  Batch(const size_t buffer_size) : tiles(buffer_size), fills(buffer_size) {}

  /**
   * @brief Clears the batch data.
   */
  inline void clear()
  {
    tiles.clear();
    fills.clear();
    data.clear();
  }
};

/**
 * @brief Represents the options to outline a path.
 */
struct Outline {
  const std::unordered_set<uint32_t>* selected_vertices;  // The selected vertices, can be nullptr.
  bool draw_vertices;  // Whether to draw individual the vertices.
  vec4 color;          // The color of the outline.
};

/**
 * @brief Represents the options to draw a path: fill, stroke and outline.
 */
struct DrawingOptions {
  const Fill* fill;        // The fill to use, can be nullptr.
  const Stroke* stroke;    // The stroke to use, can be nullptr.
  const Outline* outline;  // The outline to use, can be nullptr.
};

/**
 * @brief Collects all the UI related options, transformed based on the viewport.
 */
struct UIOptions {
  vec2 vertex_size;        // The size of a vertex.
  vec2 vertex_inner_size;  // The size of the white part of a vertex.

  float handle_radius;     // The radius of an handle.
  float line_width;        // The default width of the lines.

  vec4 primary_color;      // The primary color of the UI.
  vec4 primary_color_05;   // The primary color 5% darker.

  /**
   * @brief Constructs a new UIOptions object.
   *
   * @param factor The factor to scale the options with (dpr / zoom).
   */
  UIOptions(const double factor)
      : vertex_size(vec2(RendererSettings::ui_handle_size * factor)),
        vertex_inner_size(vec2((RendererSettings::ui_handle_size - 2.0) * factor)),
        handle_radius(static_cast<float>(RendererSettings::ui_handle_size * factor / 2.0)),
        line_width(static_cast<float>(RendererSettings::ui_line_width)),
        primary_color(RendererSettings::ui_primary_color),
        primary_color_05(RendererSettings::ui_primary_color * 0.95f)
  {
  }

  UIOptions() : UIOptions(1.0) {}
};

}  // namespace graphick::renderer
