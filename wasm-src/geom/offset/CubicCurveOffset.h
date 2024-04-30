
#pragma once

#include "../cubic_bezier.h"
#include "../curve_ops.h"
#include "../line.h"

namespace graphick::geom {

#define ARRAY_SIZE(a) \
    ((sizeof(a) / sizeof(*(a))) / \
      static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

#define SIZE_OF(a) \
    (static_cast<int>(sizeof(a)))

#define DISABLE_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName &) = delete; \
    void operator=(const TypeName &) = delete;

#define ASSERT(x)

  /**
   * Used for parallel curve construction.
   */
  class CubicCurveBuilder final {
  public:
    CubicCurveBuilder(quadratic_path& path) : m_path(path) {
    }
  public:
    /**
     * Adds line.
     */
    void AddLine(const dvec2 p0, const dvec2 p1);

    void AddQuadratic(const dvec2 p0, const dvec2 cp, const dvec2 to);

    /**
     * Adds cubic curve.
     */
    void AddCubic(const dvec2 p0, const dvec2 cp1,
        const dvec2 cp2, const dvec2 to);
  private:
    quadratic_path& m_path;
  };

  inline void CubicCurveBuilder::AddLine(const dvec2 p0, const dvec2 p1) {
    m_path.line_to(vec2(p1));
  }

  inline void CubicCurveBuilder::AddQuadratic(const dvec2 p0, const dvec2 cp, const dvec2 to) {
    m_path.quadratic_to(vec2(cp), vec2(to));
  }

  inline void CubicCurveBuilder::AddCubic(const dvec2 p0, const dvec2 cp1, const dvec2 cp2, const dvec2 to) {
    geom::cubic_to_quadratics(cubic_bezier{ vec2(p0), vec2(cp1), vec2(cp2), vec2(to) }, 2e-2f, m_path);
  }

  /**
   * Find a set of segments that approximate parallel curve.
   *
   * @param curve Input curve.
   *
   * @param offset Offset amount. If it is zero, resulting curve will be
   * identical to input curve. Can be negative.
   *
   * @param maximumError Maximum error. Lower value means better precision and
   * more output segments. Larger value means worse precision, but fewer output
   * segments.
   *
   * @param builder Output receiver.
   */
  extern void OffsetCurve(const dcubic_bezier& curve, const double offset,
      const double maximumError, CubicCurveBuilder& builder);

}
