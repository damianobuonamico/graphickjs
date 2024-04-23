/**
 * @file geom/path.cpp
 * @brief Implementation of the Path class.
 *
 * @todo implement compound paths when groups are a thing.
 */

#include "path.h"

#include "../math/vector.h"

namespace graphick::geom {

  /* -- Segment -- */

  template <typename T, typename _>
  bool Path<T, _>::Segment::is_point() const {
    bool point = math::is_almost_equal(p0, p1);

    if (point) {
      if (is_quadratic()) return math::is_almost_equal(p1, p2);
      if (is_cubic()) return math::is_almost_equal(p1, p2) && math::is_almost_equal(p2, p3);
    }

    return point;
  }

  /* -- Template Instantiation -- */

  template Path<float>;
  template Path<double>;
}