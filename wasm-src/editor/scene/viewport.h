/**
 * @file viewport.h
 * @brief Contains the declaration of the Viewport class.
 *
 * @todo viewport encoding and decoding
 */

#pragma once

#include "../../math/vec2.h"
#include "../../math/rect.h"

namespace graphick::editor {

  /**
   * @brief The Viewport class represents the viewport (or camera) of the scene.
   *
   * Each scene has a viewport that is used to determine which part of the scene is visible to the user.
   *
   * @class Viewport
   */
  class Viewport {
  public:
    /**
     * @brief Default constructor.
     */
    Viewport();

    /**
     * @brief Construct a new Viewport class.
     *
     * @param position The position of the viewport.
     * @param zoom The zoom level of the viewport.
     * @param rotation The rotation of the viewport.
     */
    Viewport(const vec2 position, float zoom, float rotation);

    /**
     * @brief Default destructor.
    */
    ~Viewport() = default;

    /**
     * @brief Get the position of the viewport.
     *
     * @return The position of the viewport.
     */
    inline vec2 position() const { return m_position; }

    /**
     * @brief Get the zoom level of the viewport.
     *
     * @return The zoom level of the viewport.
     */
    inline float zoom() const { return m_zoom; }

    /**
     * @brief Get the size of the viewport.
     *
     * @return The size of the viewport.
     */
    inline ivec2 size() const { return m_size; }

    /**
     * @brief Get the device-pixel-ratio of the viewport.
     *
     * @return The device-pixel-ratio of the viewport.
     */
    inline float dpr() const { return m_dpr; }

    /**
     * @brief Calculates the scene-space rectangle that is visible in the viewport.
     *
     * @return The scene-space rectangle that is visible in the viewport.
     */
    inline rect visible() const { return { -m_position, vec2{ (float)m_size.x, (float)m_size.y } / m_zoom - m_position }; }

    /**
     * @brief Resizes the viewport to the given size.
     *
     * @param size The new size of the viewport.
     * @param offset The new offset of the viewport.
     * @param dpr The new device-pixel-ratio of the viewport.
     */
    void resize(const ivec2 size, const ivec2 offset, float dpr);

    /**
     * @brief Moves the viewport by the given amount.
     *
     * @param movement The amount to move the viewport by.
     */
    void move(const vec2 movement);

    /**
     * @brief Moves the viewport to the given position.
     *
     * @param position The position to move the viewport to.
     */
    void move_to(const vec2 position);

    /**
     * @brief Zooms the viewport to the given zoom level.
     *
     * @param zoom The zoom level to zoom the viewport to.
     */
    void zoom_to(float zoom);

    /**
     * @brief Zooms the viewport to the given zoom level.
     *
     * This method automatically adjusts the position of the viewport to keep the given zoom_origin in the same position.
     *
     * @param zoom The zoom level to zoom the viewport to.
     * @param zoom_origin The origin of the zoom.
     */
    void zoom_to(float zoom, const vec2 zoom_origin);

    /**
     * @brief Sets the bounds of the viewport.
     *
     * The bounds of the viewport are used to limit the movement of the viewport.
     *
     * @param bounds The new bounds of the viewport.
     */
    void set_bounds(const rect& bounds);

    /**
     * @brief Determines if the given rectangle is visible in the viewport.
     */
    bool is_visible(const rect& rect);

    /**
     * @brief Converts a position from client-space to scene-space.
     *
     * @param position The position to convert.
     * @return The converted position.
     */
    vec2 client_to_scene(const vec2 position);

    /**
     * @brief Converts a position from scene-space to client-space.
     *
     * @param position The position to convert.
     * @return The converted position.
     */
    vec2 scene_to_client(const vec2 position);
  private:
    /**
     * @brief Converts a position from client-space to scene-space with the given zoom level.
     *
     * @param position The position to convert.
     * @param zoom_override The zoom level to use for the conversion.
     */
    vec2 client_to_scene(const vec2 position, float zoom_override);

    /**
     * @brief Converts a position from scene-space to client-space with the given zoom level.
     *
     * @param position The position to convert.
     * @param zoom_override The zoom level to use for the conversion.
     */
    vec2 scene_to_client(const vec2 position, float zoom_override);
  private:
    ivec2 m_size;                                                 /* The size of the viewport. */
    ivec2 m_offset;                                               /* The offset of the viewport. */
    float m_dpr;                                                  /* The device-pixel-ratio of the viewport. */

    vec2 m_position;                                              /* The position of the viewport. */
    float m_zoom;                                                 /* The zoom level of the viewport. */
    float m_rotation;                                             /* The rotation of the viewport. */

    vec2 m_min_position = std::numeric_limits<vec2>::lowest();    /* The minimum position of the viewport. */
    vec2 m_max_position = std::numeric_limits<vec2>::max();       /* The maximum position of the viewport. */
    float m_min_zoom{ 0.01f };                                    /* The minimum zoom level of the viewport. */
  };

}
