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

enum class CurvesType : uint8_t {
  None = 0,  // Treat as a fill
  Quadratic = 1,
  Cubic = 2,
};

/**
 * @brief Represents a vertex used by the tile shader (40 bytes).
 *
 *
 * @note In attr 1, 7 bits for paint type are probably too much.
 * @note In attr_2, paint_coord is 10 bits instead of 8
 * @note In attr_3, there are a few wasted bits (16 bits is more than enough for both)
 */
struct TileVertex {
  vec2 position;          // | position.x (32) | position.y (32) |
  uvec4 color;            // | color.rgba (32) |
  vec2 tex_coord;         // | tex_coord.x (32) - tex_coord.y (32) |
  vec2 tex_coord_curves;  // | tex_coord_curves.x (32) - tex_coord_curves.y (32) |
  uint32_t attr_1;        // | blend (5) - paint_type (7) - curves_offset (20) |
  uint32_t attr_2;        // | z_index (20) - curves_type (2) - is_eodd (1) - paint_coord (9) |
  uint32_t attr_3;        // | winding (16) - curves_count (16) |

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
        tex_coord(tex_coord),
        tex_coord_curves(tex_coord_curves),
        attr_1(attr_1),
        attr_2(attr_2),
        attr_3(attr_3)
  {
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
    const uint32_t u_curves_offset = static_cast<uint32_t>(curves_start_index) << 12 >> 12;
    const uint32_t u_paint_type = static_cast<uint32_t>(paint_type) << 25 >> 25;
    const uint32_t u_blend = static_cast<uint32_t>(blending_mode);

    return (u_blend << 27) | (u_paint_type << 20) | (u_curves_offset);
  }

  /**
   * @brief Creates the attr_2 attribute of the vertex.
   *
   * This method should be called once per object.
   *
   * @param z_index The z-index of the vertex.
   * @param curves_type The type of curves used by the vertex.
   * @param is_even_odd Whether the fill rule is even-odd or non-zero.
   * @param paint_coord The paint coordinate of the vertex.
   * @return The attr_2 attribute.
   */
  static uint32_t create_attr_2(const uint32_t z_index,
                                const CurvesType curves_type,
                                const bool is_even_odd,
                                const uint16_t paint_coord)
  {
    const uint32_t u_curves_type = static_cast<uint32_t>(curves_type);
    const uint32_t u_is_eodd = static_cast<uint32_t>(is_even_odd);
    const uint32_t u_paint_coord = (static_cast<uint32_t>(paint_coord) << 23) >> 23;

    return (z_index << 12) | (u_curves_type << 10) | (u_is_eodd << 9) | u_paint_coord;
  }

  /**
   * @brief Creates the attr_3 attribute of the vertex.
   *
   * This method should be called once per object.
   *
   * @param winding The winding number on the left side of the tile.
   * @param curves_count The number of curve segments in the tile.
   * @return The attr_3 attribute.
   */
  static uint32_t create_attr_3(const int winding, const uint16_t curves_count)
  {
    const uint32_t u_winding = static_cast<uint32_t>((winding + 32768)) << 16 >> 16;
    const uint32_t u_curves_count = static_cast<uint32_t>(curves_count) << 16 >> 16;

    return (u_winding << 16) | u_curves_count;
  }

  /**
   * @brief Adds an offset to the curves texture coordinates.
   *
   * @param offset The offset to add.
   */
  inline void add_offset_to_curves(const size_t offset)
  {
    const uint32_t u_curves_offset = ((attr_1 & 0xFFFFFU) + offset) << 12 >> 12;
    attr_1 = (attr_1 >> 20 << 20) | (u_curves_offset);
  }

  /**
   * @brief Adds an offset to the bands texture coordinates.
   *
   * @param offset The offset to add.
   */
  inline void add_offset_to_bands(const size_t offset)
  {
    const uint32_t u_bands_offset = ((attr_3 & 0xFFFFFFU) + offset) << 8 >> 8;
    attr_3 = (attr_3 >> 24 << 24) | (u_bands_offset);
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
 * @brief Represents a vertex used by the fill shader (28 bytes).
 *
 *
 * @note In attr 1, 7 bits for paint type are probably too much.
 * @note In attr_1, there are 20 bits of padding
 * @note In attr_2, paint_coord is 10 bits instead of 8 and there are 2 bits of padding
 */
struct FillVertex {
  vec2 position;    // | position.x (32) | position.y (32) |
  uvec4 color;      // | color.rgba (32) |
  vec2 tex_coord;   // | tex_coord.x (32) - tex_coord.y (32) |
  uint32_t attr_1;  // | blend (5) - paint_type (7) - (20) |
  uint32_t attr_2;  // | z_index (20) - (2) - paint_coord (10) |

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
      : position(position), color(color), tex_coord(tex_coord), attr_1(attr_1), attr_2(attr_2)
  {
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

// struct Fill {
//   vec2 min;
//   vec2 max;

//   std::array<vec2, 4> tex_coords;
// };

// struct Tile : public Fill {
//   vec2 tex_coord_curve_min;
//   vec2 tex_coord_curve_max;

//   uint16_t curves_count;
//   int16_t winding;
// };

/**
 * @brief The Drawable class is the only object that can be directly drawn by the renderer.
 */
struct Drawable {
  uint8_t LOD;                               // The level of detail of the drawable.
  drect bounding_rect;                       // The bounding rectangle of the drawable.
  drect valid_rect;                          // The rect where the drawable is always valid.

  std::vector<vec2> curves;                  // The curves of the drawable.

  std::vector<TileVertex> tiles;             // The tiles of the drawable.
  std::vector<FillVertex> fills;             // The fills of the drawable.

  std::vector<DrawablePaintBinding> paints;  // The paint bindings of the drawable.

  Appearance appearance;                     // The appearance of the drawable.

  inline void push_curve(const vec2 p0, const vec2 p1, const vec2 p2)
  {
    curves.insert(curves.end(), {p0, p1, p2, vec2::zero()});
  }

  inline void push_curve(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3)
  {
    curves.insert(curves.end(), {p0, p1, p2, p3});
  }

  inline void push_tile(const vec2 min,
                        const vec2 max,
                        const vec2 tex_coord_curve_min,
                        const vec2 tex_coord_curve_max,
                        const std::array<vec2, 4>& tex_coords,
                        const uvec4 color,
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
