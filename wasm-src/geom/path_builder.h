/**
 * @file geom/path_builder.h
 * @brief Containes the PathBuilder class definition and its options.
 */

#pragma once

#include "cubic_path.h"
#include "options.h"

#include "../math/mat2x3.h"
#include "../math/rect.h"

#include <functional>

namespace graphick::geom {

/* -- Forward Declarations -- */

template<typename T, typename>
class Path;

template<typename T, typename>
class QuadraticPath;

/**
 * @brief A structure to hold the inner and outer outlines of a stroke.
 */
template<typename T>
struct StrokeOutline {
  geom::CubicMultipath<T> path;  // The outline of the stroke.
  math::Rect<T> bounding_rect;   // The bounding rectangle of the stroke.
};

/**
 * @brief The PathBuilder class is used to build strokes and flatten paths.
 */
template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
class PathBuilder {
 public:
  /**
   * @brief Constructs a PathBuilder from a quadratic path.
   *
   * @param path The quadratic path to process.
   * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
   */
  PathBuilder(const QuadraticPath<T, std::enable_if<true>>& path,
              const math::Rect<T>& bounding_rect);

  /**
   * @brief Constructs a PathBuilder from a cubic path.
   *
   * @param path The cubic path to process.
   * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
   */
  PathBuilder(const CubicPath<T>& path, const math::Rect<T>& bounding_rect);

  /**
   * @brief Constructs a PathBuilder from a path.
   *
   * @param path The path to process.
   * @param bounding_rect The bounding rectangle of the path if known, default is nullptr.
   */
  PathBuilder(const Path<T, std::enable_if<true>>& path, const math::Rect<T>& bounding_rect);

  /**
   * @brief Flattens a path and outputs the line segments to a sink vector.
   *
   * If the portion of the path visible is less than 50%, it is clipped.
   * This method is only available for generic paths.
   *
   * @param clip The rectangle to clip the path to after transformation.
   * @param tolerance The tolerance to use when flattening the path.
   * @param sink_callback The callback to output the lines to.
   */
  template<typename U>
  void flatten(const math::Rect<T>& clip,
               const T tolerance,
               std::function<void(const math::Vec2<U>, const math::Vec2<U>)> sink_callback) const;

  /**
   * @brief Strokes a path and outputs the resulting cubic curves grouped in contours.
   *
   * This method is only available for generic paths.
   *
   * @param options The options to use when stroking the path.
   * @param visible The portion of the path visible, curves outside will be treated as lines.
   * @return The resulting cubic curves grouped in contours.
   */
  StrokeOutline<T> stroke(const StrokingOptions<T>& options,
                          const math::Rect<T>* visible = nullptr) const;

 private:
  /**
   * @brief Clips and flattens a path and outputs the line segments to a sink vector.
   *
   * This method should only be called by the flatten() method (does not check for empty or
   * specific path types).
   *
   * @param clip The rectangle to clip the path to.
   * @param tolerance_sq The squared tolerance to use when flattening the path.
   * @param sink_callback The callback to output the lines to.
   */
  template<typename U>
  void flatten_clipped(
      const drect& clip,
      const double tolerance_sq,
      std::function<void(const math::Vec2<U>, const math::Vec2<U>)> sink_callback) const;

  /**
   * @brief Flattens a path and outputs the line segments to a sink vector.
   *
   * This method should only be called by the flatten() method (does not check for empty or
   * specific path types).
   *
   * @param tolerance The tolerance to use when flattening the path.
   * @param sink_callback The callback to output the lines to.
   */
  template<typename U>
  void flatten_unclipped(
      const double tolerance,
      std::function<void(const math::Vec2<U>, const math::Vec2<U>)> sink_callback) const;

 private:
  /**
   * @brief The type of path to process.
   */
  enum class PathType { Quadratic, Cubic, Generic };

 private:
  const union {
    const QuadraticPath<T, std::enable_if<true>>*
        m_quadratic_path;                                 // The quadratic path to process.
    const CubicPath<T>* m_cubic_path;                     // The cubic path to process.
    const Path<T, std::enable_if<true>>* m_generic_path;  // The generic path to process.
  };

  const PathType m_type;                                  // The type of the path.
  const drect m_bounding_rect;                            // The bounding rectangle of the path.
};

}  // namespace graphick::geom

/* -- Aliases -- */

namespace graphick::geom {

using path_builder = PathBuilder<float>;
using dpath_builder = PathBuilder<double>;

}  // namespace graphick::geom
