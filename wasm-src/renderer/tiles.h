/**
 * @file renderer/tiles.h
 * @brief The file contains the definition of the Tiler class.
 */

#pragma once

#include "../geom/cubic_bezier.h"
#include "../geom/cubic_path.h"

#include "../math/mat4.h"

#include "gpu/render_state.h"

#include "drawable.h"

#include <unordered_set>

namespace graphick::renderer::GPU {

struct TileProgram;
struct FillProgram;
struct TileVertexArray;
struct FillVertexArray;

}  // namespace graphick::renderer::GPU

namespace graphick::renderer {

/**
 * @brief Reprojects the texture coordinates of a rectangle to a new clipped rectangle.
 *
 * @param bounding_rect The bounding rectangle of the path.
 * @param clipped_rect The portion of the bounding rectangle that is needed.
 * @param texture_coords The texture coordinates of original rectangle.
 * @param r_tex_coord_curves_min The minimum texture coordinate of the clipped rectangle.
 * @param r_tex_coord_curves_max The maximum texture coordinate of the clipped rectangle.
 * @return The reprojected texture coordinates.
 */
std::array<vec2, 4> reproject_texture_coords(const drect bounding_rect,
                                             const drect clipped_rect,
                                             const std::array<vec2, 4>& texture_coords,
                                             vec2& r_tex_coord_curves_min,
                                             vec2& r_tex_coord_curves_max);

/**
 * @brief Reprojects the texture coordinates of a rectangle to a new clipped rectangle.
 *
 * @param bounding_rect The bounding rectangle of the path.
 * @param clipped_rect The portion of the bounding rectangle that is needed.
 * @param texture_coords The texture coordinates of original rectangle.
 * @return The reprojected texture coordinates.
 */
std::array<vec2, 4> reproject_texture_coords(const drect bounding_rect,
                                             const drect clipped_rect,
                                             const std::array<vec2, 4>& texture_coords);

class Tiler {
 public:
  /**
   * @brief Default constructor.
   */
  Tiler();

  void setup(const double zoom);

  /**
   * @brief Returns the largest scene-space tile size.
   *
   * @return The base tile size.
   */
  inline double base_tile_size() const
  {
    return m_base_cell_size;
  }

  /**
   * @brief Returns the maximum level of subdivisions of a base tile.
   *
   * @return The maximum level of detail.
   */
  inline uint8_t LOD() const
  {
    return m_LOD;
  }

  /**
   * @brief Adds the tiles of a cubic path to the drawable.
   *
   * @param path The cubic path to tile.
   * @param bounding_rect The bounding rectangle of the path.
   * @param fill The fill of the path.
   * @param texture_coords The texture coordinates of the fill.
   * @param drawable The drawable to add the tiles to.
   */
  void tile(const geom::dcubic_path& path,
            const drect& bounding_rect,
            const Fill& fill,
            const std::array<vec2, 4>& texture_coords,
            Drawable& drawable);

 private:
  /**
   * @brief The Cell struct represents a cell of the tiling grid.
   */
  struct Cell {
    uint16_t curves_count;   // The number of curves in the cell.
    uint32_t curves_offset;  // If zero, the curves are in
    // std::vector<uint16_t> curves;  // The curves indices in the cell.
  };

  /**
   * @brief The Intersection struct represents an intersection of a segment with a tile.
   */
  struct Intersection {
    double x;     // The x-coordinate of the intersection.
    int8_t sign;  // Whether the segment is going up or down.
  };

  using Intersections = std::vector<Intersection>;

  struct CellRows {
   public:
    inline const std::unordered_set<uint16_t>& operator[](const size_t index) const
    {
      return m_rows[index];
    }

    inline const Intersections& intersections(const size_t index) const
    {
      return m_intersections[index];
    }

    inline Intersections& intersections(const size_t index)
    {
      return m_intersections[index];
    }

    inline size_t size() const
    {
      return m_size;
    }

    inline void clear_and_resize(const int x, const int y)
    {
      for (size_t i = 0; i < std::min(size_t(y), m_capacity); i++) {
        m_rows[i].clear();
        m_intersections[i].clear();
      }

      if (y > m_capacity) {
        m_rows.resize(y);
        m_intersections.resize(y);
        m_capacity = y;
      }

      m_hsize = x;
      m_size = y;

      m_tiles.clear();
      m_tiles.resize(x * y, false);
    }

    inline void insert(const int x, const int y, const uint16_t curve)
    {
      m_rows[y].insert(curve);
      m_tiles[y * m_hsize + x] = true;
    }

    inline void intersection(const int y, const Intersection i)
    {
      m_intersections[y].push_back(i);
    }

    inline bool is_tile(const int x, const int y) const
    {
      return m_tiles[y * m_hsize + x];
    }

   private:
    std::vector<std::unordered_set<uint16_t>> m_rows;
    std::vector<Intersections> m_intersections;
    std::vector<bool> m_tiles;

    size_t m_capacity = 0;
    size_t m_hsize = 0;
    size_t m_size = 0;
  };

 private:
  drect m_visible;                                 // The visible area of the scene.

  double m_zoom;                                   // The current zoom level.
  double m_base_cell_size;                         // The largest scene-space tile size.
  double m_cell_size;                              // The current (smallest) scene-space tile size.

  uint8_t m_LOD;                                   // The maximum level of detail.
  ivec2 m_cell_count;                              // The number of tiles in the x and y direction.

  CellRows m_cells;                                // Rows of cells of the path being tiled.

  std::unordered_map<int, uint32_t> m_curves_map;  // The map of curves group to indices.
};

/**
 * @brief Represents the data of a single tile batch.
 */
struct TileBatchData {
  size_t max_vertices;          // The maximum number of vertices in the batch.
  size_t max_indices;           // The maximum number of indices in the batch.
  size_t max_curves;            // The maximum number of control points in the curves texture.

  TileVertex* vertices;         // The vertices of the batch.
  TileVertex* vertices_ptr;     // The current index of the vertices.

  uint16_t* indices;            // The indices of the batch, these are all static quads.
  uint16_t* indices_ptr;        // The current index of the indices.

  vec2* curves;                 // The control points of the curves.
  vec2* curves_ptr;             // The current index of the curves.

  GPU::Buffer vertex_buffer;    // The GPU vertex buffer.
  GPU::Buffer index_buffer;     // The GPU index buffer.
  GPU::Texture curves_texture;  // The curves texture.

  GPU::Primitive primitive;     // The primitive type of the mesh.

#define GK_CURVES_TEXTURE_SIZE 256

  /**
   * @brief Constructs a new TileBatchData object.
   */
  TileBatchData(const size_t buffer_size)
      : primitive(GPU::Primitive::Triangles),
        max_vertices(buffer_size / sizeof(TileVertex)),
        max_indices(max_vertices * 3 / 2),
        max_curves(GK_CURVES_TEXTURE_SIZE * GK_CURVES_TEXTURE_SIZE),
        vertex_buffer(GPU::BufferTarget::Vertex,
                      GPU::BufferUploadMode::Dynamic,
                      max_vertices * sizeof(TileVertex)),
        index_buffer(GPU::BufferTarget::Index,
                     GPU::BufferUploadMode::Static,
                     max_indices * sizeof(uint16_t)),
        curves_texture(GPU::TextureFormat::RGBA32F,
                       ivec2{GK_CURVES_TEXTURE_SIZE},
                       GPU::TextureSamplingFlagNearestMin | GPU::TextureSamplingFlagNearestMag)
  {
    vertices = new TileVertex[max_vertices];
    indices = new uint16_t[max_indices];
    curves = new vec2[max_curves * 2];

    vertices_ptr = vertices;
    indices_ptr = indices;
    curves_ptr = curves;

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
   * @brief Clears the batch data.
   */
  inline void clear()
  {
    vertices_ptr = vertices;
    indices_ptr = indices;
    curves_ptr = curves;
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
   * @brief Uploads vertices and curves to the buffers.
   *
   * @param drawable The drawable with the tile vertices and curves to upload.
   * @param z_index The z-index of the drawable.
   */
  inline void upload(const Drawable& drawable, const uint32_t z_index)
  {
    memcpy(vertices_ptr, drawable.tiles.data(), drawable.tiles.size() * sizeof(TileVertex));
    memcpy(curves_ptr, drawable.curves.data(), drawable.curves.size() * sizeof(vec2));

    const size_t curves_start_index = curves_count();

    const TileVertex* vertices_end_ptr = vertices_ptr + drawable.tiles.size();

    for (; vertices_ptr < vertices_end_ptr; vertices_ptr++) {
      vertices_ptr->add_offset_to_curves(curves_start_index);
      vertices_ptr->update_z_index(z_index);
    }

    curves_ptr += drawable.curves.size();
  }

  /**
   * @brief Uploads vertices and curves to the buffers.
   *
   * @param drawable The drawable with the tile vertices and curves to upload.
   * @param z_index The z-index of the drawable.
   * @param textures The texture bindings to use for the drawable.
   */
  inline void upload(const Drawable& drawable,
                     const uint32_t z_index,
                     const std::vector<std::pair<uuid, uint32_t>>& textures)
  {
    memcpy(vertices_ptr, drawable.tiles.data(), drawable.tiles.size() * sizeof(TileVertex));
    memcpy(curves_ptr, drawable.curves.data(), drawable.curves.size() * sizeof(vec2));

    const size_t curves_start_index = curves_count();

    size_t local_z_index = z_index;

    const TileVertex* vertices_start_ptr = vertices_ptr;

    for (const DrawablePaintBinding& binding : drawable.paints) {
      const TileVertex* vertices_end_ptr = vertices_start_ptr + binding.last_tile_index;

      if (binding.paint_type == Paint::Type::TexturePaint) {
        const auto it = std::find_if(
            textures.begin(), textures.end(), [binding](const auto& pair) {
              return pair.first == binding.paint_id;
            });

        if (it == textures.end()) {
          continue;
        }

        const uint32_t texture_index = it->second;

        for (; vertices_ptr < vertices_end_ptr; vertices_ptr++) {
          vertices_ptr->add_offset_to_curves(curves_start_index);
          vertices_ptr->update_z_index(local_z_index);
          vertices_ptr->update_paint_coord(texture_index);
        }
      } else {
        for (; vertices_ptr < vertices_end_ptr; vertices_ptr++) {
          vertices_ptr->add_offset_to_curves(curves_start_index);
          vertices_ptr->update_z_index(local_z_index);
        }
      }

      local_z_index++;
    }

    curves_ptr += drawable.curves.size();
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
   * @brief Uploads vertices and curves to the buffers.
   *
   * @param drawable The drawable with the tile vertices and curves to upload.
   * @param z_index The z-index of the drawable.
   * @param textures The texture bindings to use for the drawable.
   */
  inline void upload(const Drawable& drawable,
                     const uint32_t z_index,
                     const std::vector<std::pair<uuid, uint32_t>>& textures)
  {
    memcpy(vertices_ptr, drawable.fills.data(), drawable.fills.size() * sizeof(FillVertex));

    uint32_t local_z_index = z_index;

    const FillVertex* vertices_start_ptr = vertices_ptr;

    for (const DrawablePaintBinding& binding : drawable.paints) {
      const FillVertex* vertices_end_ptr = vertices_start_ptr + binding.last_fill_index;

      if (binding.paint_type == Paint::Type::TexturePaint) {
        const auto it = std::find_if(
            textures.begin(), textures.end(), [binding](const auto& pair) {
              return pair.first == binding.paint_id;
            });

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

#define GK_GRADIENTS_TEXTURE_WIDTH 64
#define GK_GRADIENTS_TEXTURE_HEIGHT 64

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

class TiledRenderer {
 public:
  /**
   * @brief Constructs a new TiledRenderer object.
   */
  TiledRenderer(const size_t buffer_size) : m_batch(buffer_size) {}

  /**
   * @brief Sets up the renderer with the given zoom level and visible area.
   *
   * @param viewport_size The size of the viewport.
   * @param visible The visible area of the scene.
   * @param vp_matrix The view projection matrix.
   * @param LOD The level of detail.
   * @param base_cell_size The base cell size.
   */
  void setup(const ivec2 viewport_size,
             const drect& visible,
             const mat4& vp_matrix,
             const uint8_t LOD,
             const double base_cell_size);

  /**
   * @brief Updates the shader and vertex array to use.
   *
   * @param tile_program The program to use for tiles.
   * @param fill_program The program to use for fills.
   * @param tile_vertex_array The vertex array to use with tiles.
   * @param fill_vertex_array The vertex array to use with fills.
   * @param textures The textures to use.
   */
  inline void update_shaders(GPU::TileProgram* tile_program,
                             GPU::FillProgram* fill_program,
                             GPU::TileVertexArray* tile_vertex_array,
                             GPU::FillVertexArray* fill_vertex_array,
                             std::unordered_map<uuid, GPU::Texture>* textures)
  {
    m_tile_program = tile_program;
    m_fill_program = fill_program;
    m_tile_vertex_array = tile_vertex_array;
    m_fill_vertex_array = fill_vertex_array;
    m_textures = textures;
  }

  /**
   * @brief Returns the tiles vertex buffer.
   *
   * @return The tiles vertex buffer.
   */
  inline const GPU::Buffer& tiles_vertex_buffer() const
  {
    return m_batch.tiles.vertex_buffer;
  }

  /**
   * @brief Returns the tiles index buffer.
   *
   * @return The tiles index buffer.
   */
  inline const GPU::Buffer& tiles_index_buffer() const
  {
    return m_batch.tiles.index_buffer;
  }

  /**
   * @brief Returns the fills vertex buffer.
   *
   * @return The fills vertex buffer.
   */
  inline const GPU::Buffer& fills_vertex_buffer() const
  {
    return m_batch.fills.vertex_buffer;
  }

  /**
   * @brief Returns the fills index buffer.
   *
   * @return The fills index buffer.
   */
  inline const GPU::Buffer& fills_index_buffer() const
  {
    return m_batch.fills.index_buffer;
  }

  /**
   * @brief Adds a new drawable to the batch.
   *
   * Drawables should be pushed back to front.
   *
   * @param drawable A pointer to the cached drawable.
   */
  void push_drawable(const Drawable* drawable);

  /**
   * @brief Flushes the instanced data to the GPU.
   *
   * Here the GPU draw calls are actually issued.
   */
  void flush();

 private:
  /**
   * @brief Populates the fill batches.
   */
  void populate_fills();

  /**
   * @brief Renders the tile batches.
   */
  void render_tiles();

 private:
  Batch m_batch;            // The tile/fill batch to render.
  uint32_t m_z_index;       // The current z-index.

  ivec2 m_viewport_size;    // The size of the viewport.
  drect m_visible;          // The visible area of the scene.
  mat4 m_vp_matrix;         // The view projection matrix.

  uint8_t m_LOD;            // The level of detail.
  ivec2 m_cell_count;       // The number of tiles in the x and y direction.

  double m_base_cell_size;  // The base cell size.
  double m_cell_sizes[3];   // The cell sizes for each valid LOD (-1, m_LOD, +1).

  GPU::Framebuffer* m_front_framebuffer = nullptr;                  // The front framebuffer.
  GPU::Framebuffer* m_back_framebuffer = nullptr;                   // The back framebuffer.

  std::vector<std::pair<const Drawable*, uint32_t>> m_front_stack;  // Draw now.
  std::vector<std::pair<const Drawable*, uint32_t>> m_back_stack;   // Draw next call.

  std::vector<uint32_t> m_culled;                                   // The culled tiles.
  std::vector<bool> m_semivalid;                                    // Can draw normal tiles.
  std::vector<bool> m_invalid;                                      // Cannot draw anything.

  GPU::TileVertexArray* m_tile_vertex_array;                 // The tile vertex array to use.
  GPU::FillVertexArray* m_fill_vertex_array;                 // The fill vertex array to use.
  GPU::TileProgram* m_tile_program;                          // The tile program to use.
  GPU::FillProgram* m_fill_program;                          // The fill program to use.

  std::unordered_map<uuid, GPU::Texture>* m_textures;        // The textures loaded in the GPU.
  std::vector<std::pair<uuid, uint32_t>> m_binded_textures;  // The textures bound to the GPU.
};

}  // namespace graphick::renderer
