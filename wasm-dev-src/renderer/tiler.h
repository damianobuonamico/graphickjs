/**
 * @file tiler.h
 * @brief Contains the classes used to generate segments and cover tables for a collection of drawables.
 *
 * @todo cleanup and doc Tiler class
 */

#pragma once

#include "renderer_data.h"
#include "drawable.h"

#include "../math/mat2x3.h"
#include "../math/dvec2.h"
#include "../math/f8x8.h"
#include "../math/rect.h"

#include <vector>

namespace Graphick::Renderer {

  namespace Geometry {
    class Path;
    class Segment;
  }

  /**
   * @brief Class used to generate segments and cover tables for a drawable.
   *
   * @class DrawableTiler
   */
  class DrawableTiler {
  public:
    /**
     * @brief The intermediate tile object used to process segments.
     *
     * @struct Tile
     */
    struct Tile {
      bool active = false;                        /* Whether the tile is active. */
      int8_t winding = 0;                         /* Winding number increment. */

      float cover_table[TILE_SIZE] = { 0.0f };    /* Cover table of the tile. */
      std::vector<uvec4> segments;                /* Segments of the tile. */
    };

    /**
     * @brief The span used to represent a completely covered array of tiles.
     *
     * @struct Span
     */
    struct Span {
      int16_t tile_x;   /* The x coordinate of the tile. */
      int16_t tile_y;   /* The y coordinate of the tile. */

      int16_t width;    /* The width of the span. */
    };

    /**
     * @brief The mask used to represent a partially covered tile.
     *
     * @struct Mask
     */
    struct Mask {
      int16_t tile_x;                             /* The x coordinate of the tile. */
      int16_t tile_y;                             /* The y coordinate of the tile. */

      float cover_table[TILE_SIZE] = { 0.0f };    /* GPU packed cover table of the tile. */
      std::vector<uvec4> segments;                /* GPU packed segments of the tile. */
    };
  public:
    /**
     * @brief Default copy constructor, move constructor and destructor.
     */
    DrawableTiler(const DrawableTiler&) = default;
    DrawableTiler(DrawableTiler&&) = default;
    ~DrawableTiler() = default;

    /**
     * @brief Constructs a new DrawableTiler object.
     *
     * @param drawable The drawable to tile.
     * @param visible The bounds of the viewport.
     * @param zoom The zoom of the viewport.
     * @param position The position of the viewport.
     * @param subpixel The subpixel offset of the viewport.
     * @param tiles_count The dimensions in tiles of the viewport.
     * @param pool The memory pool of tiles to use.
     */
    DrawableTiler(const Drawable& drawable, const rect& visible, const float zoom, const ivec2 position, const vec2 subpixel, const ivec2 tiles_count, std::vector<Tile>& pool);

    /**
     * @brief Returns the offset of the drawable.
     *
     * @return The offset of the drawable.
     */
    inline ivec2 offset() const { return m_offset; }

    /**
     * @brief Returns the size of the drawable.
     *
     * @return The size of the drawable.
     */
    inline ivec2 size() const { return m_size; }

    /**
     * @brief Returns the masks of the drawable.
     *
     * @return The masks of the drawable.
     */
    inline const std::vector<Mask>& masks() const { return m_masks; }

    /**
     * @brief Returns the spans of the drawable.
     *
     * @return The spans of the drawable.
     */
    inline const std::vector<Span>& spans() const { return m_spans; }
  private:
    /**
     * @brief Moves the drawing cursor to the specified point.
     *
     * @param p0 The start point of the contour.
     */
    void move_to(const vec2 p0);

    /**
     * @brief Draws a line from the cursor position to the specified point.
     *
     * @param p3 The end point of the line.
     */
    void line_to(const vec2 p3);

    /**
     * @brief Finishes the tiling of the drawable.
     *
     * @param rule The fill rule used to determine how self-intersecting paths are filled.
     * @param tiles_count The dimensions in tiles of the viewport.
     */
    void pack(const FillRule rule, const ivec2 tiles_count);
  private:
    vec2 m_p0;                           /* The last point. */

    ivec2 m_offset;                      /* The tile offset of the drawable. */
    ivec2 m_size;                        /* The size in tiles of the drawable. */

    std::vector<Span> m_spans;           /* The spans used to represent the completely covered tiles of the drawable. */
    std::vector<Mask> m_masks;           /* The masks used to represent the partially covered tiles of the drawable. */

    int16_t m_tile_y_prev = 0;           /* The y coordinate of the previous tile. */

    std::vector<Tile>* m_memory_pool;    /* The memory pool of tiles to use. */
  };

#ifdef USE_F8x8

  /**
   * @brief Class used to generate segments and cover tables for a collection of drawables.
   *
   * @class Tiler
   */
  class Tiler {
  public:
    /**
     * @brief Filled tiles batch ready to be sent to the GPU.
     *
     * @struct FilledTilesBatch
     */
    struct FilledTilesBatch {
      std::vector<FilledTile> tiles;    /* The filled tiles. */
    };

    /**
     * @brief Masked tiles batch ready to be sent to the GPU.
     *
     * @struct MaskedTilesBatch
     */
    struct MaskedTilesBatch {
      std::vector<MaskedTile> tiles;    /* The masked tiles. */

      uint8_t* segments = nullptr;                     /* The segments of the tiles. */
      uint8_t* segments_ptr = nullptr;                 /* The pointer to the next segment. */
      float* cover_table = nullptr;                    /* The cover tables of the tiles. */
      float* cover_table_ptr = nullptr;                /* The pointer to the next cover table. */

      MaskedTilesBatch() {
        segments = new uint8_t[SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE * 4];
        segments_ptr = segments;
        cover_table = new float[SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE];
        cover_table_ptr = cover_table;
      }

      ~MaskedTilesBatch() {
        delete[] segments;
        delete[] cover_table;
      }
    };
  public:
    /**
     * @brief Default constructor, destructor and deleted copy constructor and move constructor.
     */
    Tiler() = default;
    ~Tiler() = default;
    Tiler(Tiler&&) = delete;
    Tiler(const Tiler&) = delete;

    /**
     * @brief Returns the opaque tiles, grouped in batches.
     *
     * @return The opaque tiles.
     */
    inline const std::vector<FilledTilesBatch>& filled_tiles_batches() const { return m_filled_batches; }

    /**
     * @brief Returns the masked tiles, grouped in batches.
     *
     * @return The masked tiles.
     */
    inline const std::vector<MaskedTilesBatch>& masked_tiles_batches() const { return m_masked_batches; }

    /**
     * @brief Resets the tiler.
     *
     * @param viewport The viewport to tile.
     */
    void reset(const Viewport& viewport);

    /**
     * @brief Processes a stroke.
     *
     * @param path The path to stroke.
     * @param transform The transform of the path.
     * @param stroke The stroke to use.
     */
    void process_stroke(const Geometry::Path& path, const mat2x3& transform, const Stroke& stroke);

    /**
     * @brief Processes a fill.
     *
     * @param path The path to fill.
     * @param transform The transform of the path.
     * @param fill The fill to use.
     */
    void process_fill(const Geometry::Path& path, const mat2x3& transform, const Fill& fill);
  private:
    /**
     * @brief Processes a drawable.
     *
     * @param drawable The drawable to process.
     * @param visible The bounds of the viewport.
     * @param offset The offset of the drawable.
     * @param clip Whether to clip the drawable.
     */
    void process_drawable(const Drawable& drawable, const rect& visible, const vec2 offset = { 0.0f, 0.0f }, const bool clip = true);
  private:
    std::vector<FilledTilesBatch> m_filled_batches;    /* The filled tiles, grouped in batches. */
    std::vector<MaskedTilesBatch> m_masked_batches;    /* The masked tiles, grouped in batches. */

    std::vector<DrawableTiler::Tile> m_memory_pool;    /* The memory pool of tiles to use. */
    std::vector<bool> m_culled_tiles;                  /* The culled tiles. */

    double m_zoom = 1.0;                               /* The zoom of the viewport. */

    ivec2 m_position = { 0, 0 };                       /* The position of the viewport. */
    ivec2 m_size = { 0, 0 };                           /* The size in tiles of the viewport. */

    vec2 m_subpixel = { 0.0f, 0.0f };                  /* The subpixel offset of the viewport. */

    rect m_visible;                                    /* The bounds of the viewport in scene space. */

    dvec2 m_visible_min;                               /* The upper left corner of the viewport. */

    // float m_zoom = 1.0f;                               /* The zoom of the viewport. */
    // ivec2 m_position = { 0, 0 };                       /* The position of the viewport. */
    // ivec2 m_tiles_count = { 0, 0 };                    /* The dimensions in tiles of the viewport. */
    // vec2 m_subpixel = { 0.0f, 0.0f };                  /* The subpixel offset of the viewport. */
    // rect m_visible;                                    /* The bounds of the viewport. */

    // uint8_t* m_segments = nullptr;                     /* The segments of the tiles. */
    // uint8_t* m_segments_ptr = nullptr;                 /* The pointer to the next segment. */
    // float* m_cover_table = nullptr;                    /* The cover tables of the tiles. */
    // float* m_cover_table_ptr = nullptr;                /* The pointer to the next cover table. */
  };
#else
  class Tiler {
  public:
    Tiler(const Tiler&) = delete;
    Tiler(Tiler&&) = delete;

    Tiler();
    ~Tiler();

    inline const std::vector<OpaqueTile>& opaque_tiles() const { return m_opaque_tiles; }
    inline const std::vector<MaskedTile>& masked_tiles() const { return m_masked_tiles; }

    inline const uint8_t* segments() const { return m_segments; };
    inline const float* cover_table() const { return m_cover_table; };
    inline const size_t segments_size() const { return m_segments_ptr - m_segments; };

    void reset(const Viewport& viewport);

    void process_drawable(const Drawable& drawable, const rect& visible, const vec2 offset = { 0.0f, 0.0f }, const bool clip = true);
    void process_stroke(const Geometry::Path& path, const mat2x3& transform, const Stroke& stroke);
    void process_fill(const Geometry::Path& path, const mat2x3& transform, const Fill& fill);
  private:
    std::vector<MaskedTile> m_masked_tiles;
    std::vector<OpaqueTile> m_opaque_tiles;
    std::vector<bool> m_culled_tiles;

    std::vector<DrawableTiler::Tile> m_memory_pool;

    float m_zoom = 1.0f;
    ivec2 m_position = { 0, 0 };
    ivec2 m_tiles_count = { 0, 0 };
    vec2 m_subpixel = { 0.0f, 0.0f };
    rect m_visible;

    // TODO: replace with array of textures
    uint8_t* m_segments = nullptr;
    uint8_t* m_segments_ptr = nullptr;
    float* m_cover_table = nullptr;
    float* m_cover_table_ptr = nullptr;
  };
#endif

}
