/**
 * @file renderer/drawable.h
 * @brief Contains the Drawable class definition and its related structs.
 *
 * @todo other types of fill (gradient, texture, etc.)
 */

#pragma once

#include "properties.h"

#include "../math/rect.h"
#include "../math/vec2.h"

#include "../utils/half.h"

namespace graphick::renderer {

/**
 * @brief Represents a vertex used by the tile shader (36 bytes).
 *
 *
 * @note In attr 1, 7 bits for paint type are probably too much.
 * @note In attr_2, paint_coord is 10 bits instead of 8
 * @note In attr_3, there are a few wasted bits (max bands is 64, but 256 is used here, bands
 * coords are 12 bits instead of 10).
 */
struct TileVertex {
  vec2 position;               // | position.x (32) | position.y (32) |
  uvec4 color;                 // | color.rgba (32) |
  math::Vec2<half> tex_coord;  // | tex_coord.x (16) - tex_coord.y (16) |
  vec2 tex_coord_curves;       // | tex_coord_curves.x (32) - tex_coord_curves.y (32) |
  uint32_t attr_1;             // | blend (5) - paint_type (7) - curves_x (10) - curves_y (10) |
  uint32_t attr_2;             // | z_index (20) - is_quad (1) - is_eodd (1) - paint_coord (10) |
  uint32_t attr_3;             // | bands_h (8) - bands_x (12) - bands_y (12) |

  /**
   * @brief Default constructor.
   */
  TileVertex() = default;

  /**
   * @brief Constructs a new TileVertex object.
   *
   * @param position The position of the vertex.
   * @param color The color of the vertex.
   * @param tex_coord The texture coordinate used for painting, can be outside the range [0, 1].
   * @param tex_coord_curves The texture coordinate used for rasterization, should be in the range
   * [0, 1].
   * @param attr_1 The attributes of the vertex, should be created using
   * TileVertex::create_attr_1()
   * @param attr_2 The attributes of the vertex, should be created using
   * TileVertex::create_attr_2()
   * @param attr_3 The attributes of the vertex, should be created using
   * TileVertex::create_attr_3()
   */
  TileVertex(const vec2 position,
             const uvec4 color,
             const vec2 tex_coord,
             const vec2 tex_coord_curves,
             const uint32_t attr_1,
             const uint32_t attr_2,
             const uint32_t attr_3)
      : position(position),
        color(color),
        tex_coord_curves(tex_coord_curves),
        attr_1(attr_1),
        attr_2(attr_2),
        attr_3(attr_3)
  {
    this->tex_coord.x = half(tex_coord.x);
    this->tex_coord.y = half(tex_coord.y);
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
  static uint32_t create_attr_1(const uint8_t blending_mode,
                                const uint8_t paint_type,
                                const size_t curves_start_index)
  {
    const uint32_t u_curves_x = (static_cast<uint32_t>(curves_start_index % GK_CURVES_TEXTURE_SIZE)
                                 << 22) >>
                                22;
    const uint32_t u_curves_y = (static_cast<uint32_t>(curves_start_index / GK_CURVES_TEXTURE_SIZE)
                                 << 22) >>
                                22;
    const uint32_t u_paint_type = (static_cast<uint32_t>(paint_type) << 25) >> 25;
    const uint32_t u_blend = static_cast<uint32_t>(blending_mode);

    return (u_blend << 27) | (u_paint_type << 20) | (u_curves_x << 10) | (u_curves_y);
  }

  /**
   * @brief Creates the attr_2 attribute of the vertex.
   *
   * This method should be called once per object.
   *
   * @param z_index The z-index of the vertex.
   * @param is_quadratic Whether the curves are quadratic or cubic.
   * @param is_even_odd Whether the fill rule is even-odd or non-zero.
   * @param paint_coord The paint coordinate of the vertex.
   * @return The attr_2 attribute.
   */
  static uint32_t create_attr_2(const uint32_t z_index,
                                const bool is_quadratic,
                                const bool is_even_odd,
                                const uint16_t paint_coord)
  {
    const uint32_t u_is_quad = static_cast<uint32_t>(is_quadratic);
    const uint32_t u_is_eodd = static_cast<uint32_t>(is_even_odd);
    const uint32_t u_paint_coord = (static_cast<uint32_t>(paint_coord) << 22) >> 22;

    return (z_index << 12) | (u_is_quad << 11) | (u_is_eodd << 10) | u_paint_coord;
  }

  /**
   * @brief Creates the attr_3 attribute of the vertex.
   *
   * This method should be called once per object.
   *
   * @param horizontal_bands The number of horizontal bands.
   * @param bands_start_index The index of the first band in the bands texture.
   * @return The attr_3 attribute.
   */
  static uint32_t create_attr_3(const uint8_t horizontal_bands, const size_t bands_start_index)
  {
    const uint32_t u_bands_x = (static_cast<uint32_t>(bands_start_index % GK_BANDS_TEXTURE_SIZE)
                                << 20) >>
                               20;
    const uint32_t u_bands_y = (static_cast<uint32_t>(bands_start_index / GK_BANDS_TEXTURE_SIZE)
                                << 20) >>
                               20;
    const uint32_t u_bands_h = (horizontal_bands < 1) ?
                                   0 :
                                   static_cast<uint32_t>(horizontal_bands - 1);

    return (u_bands_h << 24) | (u_bands_x << 12) | (u_bands_y);
  }

  /**
   * @brief Adds an offset to the curves texture coordinates.
   *
   * @param offset The offset to add.
   */
  inline void add_offset_to_curves(const size_t offset)
  {
    uint32_t curves_x = (attr_1 >> 10) & 0x3FFU;
    uint32_t curves_y = attr_1 & 0x3FFU;

    size_t index = curves_x + curves_y * GK_CURVES_TEXTURE_SIZE + offset;

    const uint32_t u_curves_x = (static_cast<uint32_t>(index % GK_CURVES_TEXTURE_SIZE) << 22) >>
                                22;
    const uint32_t u_curves_y = (static_cast<uint32_t>(index / GK_CURVES_TEXTURE_SIZE) << 22) >>
                                22;

    attr_1 = (attr_1 >> 20 << 20) | (u_curves_x << 10) | u_curves_y;
  }

  /**
   * @brief Adds an offset to the bands texture coordinates.
   *
   * @param offset The offset to add.
   */
  inline void add_offset_to_bands(const size_t offset)
  {
    uint32_t bands_x = (attr_3 >> 12) & 0xFFFU;
    uint32_t bands_y = attr_3 & 0xFFFU;

    size_t index = bands_x + bands_y * GK_BANDS_TEXTURE_SIZE + offset;

    const uint32_t u_bands_x = (static_cast<uint32_t>(index % GK_BANDS_TEXTURE_SIZE) << 20) >> 20;
    const uint32_t u_bands_y = (static_cast<uint32_t>(index / GK_BANDS_TEXTURE_SIZE) << 20) >> 20;

    attr_3 = (attr_3 >> 24 << 24) | (u_bands_x << 12) | u_bands_y;
  }

  /**
   * @brief Updates the z-index of the vertex.
   *
   * @param z_index The new z-index.
   */
  inline void update_z_index(const uint32_t z_index)
  {
    attr_2 = (attr_2 << 20 >> 20) | (z_index << 12);
  }

  /**
   * @brief Updates the paint coordinate of the vertex.
   *
   * @param paint_coord The new paint coordinate.
   */
  inline void update_paint_coord(const uint16_t paint_coord)
  {
    const uint32_t u_paint_coord = (static_cast<uint32_t>(paint_coord) << 22) >> 22;

    attr_2 = (attr_2 >> 10 << 10) | (paint_coord);
  }
};

/**
 * @brief Represents a vertex used by the fill shader (24 bytes).
 *
 *
 * @note In attr 1, 7 bits for paint type are probably too much.
 * @note In attr_1, there are 20 bits of padding
 * @note In attr_2, paint_coord is 10 bits instead of 8 and there are 2 bits of padding
 */
struct FillVertex {
  vec2 position;               // | position.x (32) | position.y (32) |
  uvec4 color;                 // | color.rgba (32) |
  math::Vec2<half> tex_coord;  // | tex_coord.x (16) - tex_coord.y (16) |
  uint32_t attr_1;             // | blend (5) - paint_type (7) - (20) |
  uint32_t attr_2;             // | z_index (20) - (2) - paint_coord (10) |

  /**
   * @brief Default constructor.
   */
  FillVertex() = default;

  /**
   * @brief Constructs a new FillVertex object.
   *
   * @param position The position of the vertex.
   * @param color The color of the vertex.
   * @param tex_coord The texture coordinate used for painting, can be outside the range [0, 1].
   * @param attr_1 The attributes of the vertex, should be created using
   * FillVertex::create_attr_1() or TileVertex::create_attr_1().
   * @param attr_2 The attributes of the vertex, should be created using
   * FillVertex::create_attr_2() or TileVertex::create_attr_2().
   */
  FillVertex(const vec2 position,
             const uvec4 color,
             const vec2 tex_coord,
             const uint32_t attr_1,
             const uint32_t attr_2)
      : position(position), color(color), attr_1(attr_1), attr_2(attr_2)
  {
    this->tex_coord.x = half(tex_coord.x);
    this->tex_coord.y = half(tex_coord.y);
  }

  /**
   * @brief Creates the attr_1 attribute of the vertex.
   *
   * This method should be called once per object.
   *
   * @param blending_mode The blend mode of the vertex.
   * @param paint_type The paint type of the vertex.
   * @return The attr_1 attribute.
   */
  static uint32_t create_attr_1(const uint8_t blending_mode, const uint8_t paint_type)
  {
    const uint32_t u_paint_type = (static_cast<uint32_t>(paint_type) << 25) >> 25;
    const uint32_t u_blend = static_cast<uint32_t>(blending_mode);

    return (u_blend << 27) | (u_paint_type << 20);
  }

  /**
   * @brief Creates the attr_2 attribute of the vertex.
   *
   * This method should be called once per object.
   *
   * @param z_index The z-index of the vertex.
   * @param paint_coord The paint coordinate of the vertex.
   * @return The attr_2 attribute.
   */
  static uint32_t create_attr_2(const uint32_t z_index, const uint16_t paint_coord)
  {
    const uint32_t u_paint_coord = (static_cast<uint32_t>(paint_coord) << 22) >> 22;

    return (z_index << 12) | u_paint_coord;
  }

  /**
   * @brief Updates the z-index of the vertex.
   *
   * @param z_index The new z-index.
   */
  inline void update_z_index(const uint32_t z_index)
  {
    attr_2 = (attr_2 << 20 >> 20) | (z_index << 12);
  }

  /**
   * @brief Updates the paint coordinate of the vertex.
   *
   * @param paint_coord The new paint coordinate.
   */
  inline void update_paint_coord(const uint16_t paint_coord)
  {
    const uint32_t u_paint_coord = (static_cast<uint32_t>(paint_coord) << 22) >> 22;

    attr_2 = (attr_2 >> 10 << 10) | (paint_coord);
  }
};

struct DrawablePaintBinding {
  size_t last_tile_index = 0;
  size_t last_fill_index = 0;
  Paint::Type paint_type;
  uuid paint_id;
};

/**
 * @brief The Drawable class is the only object that can be directly drawn by the renderer.
 */
struct Drawable {
  drect bounding_rect;            // The bounding rectangle of the drawable.

  std::vector<vec2> curves;       // The curves of the drawable.
  std::vector<uint16_t> bands;    // The bands of the drawable.

  std::vector<TileVertex> tiles;  // The tiles of the drawable.
  std::vector<FillVertex> fills;  // The fills of the drawable.

  // TODO: add z_index intervals

  std::vector<DrawablePaintBinding> paints;  // The paint bindings of the drawable.

  inline void push_curve(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3)
  {
    curves.insert(curves.end(), {p0, p1, p2, p3});
  }

  inline void push_band(const uint16_t band)
  {
    bands.push_back(band);
  }

  inline void push_tile(const drect& bounding_rect,
                        const uvec4 color,
                        const std::array<vec2, 4>& tex_coords,
                        const uint32_t attr_1,
                        const uint32_t attr_2,
                        const uint32_t attr_3)
  {
    const auto& [v0, v1, v2, v3] = bounding_rect.vertices();

    tiles.insert(
        tiles.end(),
        {TileVertex(vec2(v0), color, tex_coords[0], {0.0f, 0.0f}, attr_1, attr_2, attr_3),
         TileVertex(vec2(v1), color, tex_coords[1], {1.0f, 0.0f}, attr_1, attr_2, attr_3),
         TileVertex(vec2(v2), color, tex_coords[2], {1.0f, 1.0f}, attr_1, attr_2, attr_3),
         TileVertex(vec2(v3), color, tex_coords[3], {0.0f, 1.0f}, attr_1, attr_2, attr_3)});
  }

  inline void push_tile(const vec2 min,
                        const vec2 max,
                        const uvec4 color,
                        const vec2 tex_coord_curve_min,
                        const vec2 tex_coord_curve_max,
                        const std::array<vec2, 4>& tex_coords,
                        const uint32_t attr_1,
                        const uint32_t attr_2,
                        const uint32_t attr_3)
  {
    const vec2 v0 = min;
    const vec2 v1 = {max.x, min.y};
    const vec2 v2 = max;
    const vec2 v3 = {min.x, max.y};

    const vec2 c0 = tex_coord_curve_min;
    const vec2 c1 = {tex_coord_curve_max.x, tex_coord_curve_min.y};
    const vec2 c2 = tex_coord_curve_max;
    const vec2 c3 = {tex_coord_curve_min.x, tex_coord_curve_max.y};

    tiles.insert(tiles.end(),
                 {TileVertex(v0, color, tex_coords[0], c0, attr_1, attr_2, attr_3),
                  TileVertex(v1, color, tex_coords[1], c1, attr_1, attr_2, attr_3),
                  TileVertex(v2, color, tex_coords[2], c2, attr_1, attr_2, attr_3),
                  TileVertex(v3, color, tex_coords[3], c3, attr_1, attr_2, attr_3)});
  }

  inline void push_fill(const vec2 min,
                        const vec2 max,
                        const uvec4 color,
                        const std::array<vec2, 4>& tex_coords,
                        const uint32_t attr_1,
                        const uint32_t attr_2)
  {
    const vec2 v0 = min;
    const vec2 v1 = {max.x, min.y};
    const vec2 v2 = max;
    const vec2 v3 = {min.x, max.y};

    fills.insert(fills.end(),
                 {FillVertex(v0, color, tex_coords[0], attr_1, attr_2),
                  FillVertex(v1, color, tex_coords[1], attr_1, attr_2),
                  FillVertex(v2, color, tex_coords[2], attr_1, attr_2),
                  FillVertex(v3, color, tex_coords[3], attr_1, attr_2)});
  }
};

}  // namespace graphick::renderer
