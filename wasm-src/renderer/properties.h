/**
 * @file properties.h
 * @brief Contains the properties used to render a path.
 */

#pragma once

#include "../math/vec4.h"

#include "../geom/options.h"

#include "../utils/uuid.h"

namespace graphick::renderer {

using LineCap = geom::LineCap;
using LineJoin = geom::LineJoin;
using FillRule = geom::FillRule;

/**
 * @brief A paint can be a color, swatch, gradient, or texture.
 */
struct Paint {
 public:
  /**
   * @brief The type of the paint.
   */
  enum Type : uint8_t { ColorPaint = 0, SwatchPaint, GradientPaint, TexturePaint };

 public:
  /**
   * @brief Constructor.
   */
  Paint(const Paint& paint) : m_type(paint.m_type), m_color(paint.m_color) {}
  Paint(const vec4& color) : m_type(Type::ColorPaint), m_color(color) {}
  Paint(const uuid paint_id, const Type type) : m_type(type), m_id(paint_id)
  {
    if (m_type == Type::ColorPaint) {
      m_type = Type::SwatchPaint;
    }
  }

  /**
   * @brief Assignment operators.
   */
  Paint& operator=(const Paint& paint)
  {
    m_type = paint.m_type;
    m_color = paint.m_color;
    return *this;
  }

  /**
   * @brief Checks if the paint is a color.
   */
  inline bool is_color() const
  {
    return m_type == Type::ColorPaint;
  }

  /**
   * @brief Checks if the paint is a swatch.
   */
  inline bool is_swatch() const
  {
    return m_type == Type::SwatchPaint;
  }

  /**
   * @brief Checks if the paint is a gradient.
   */
  inline bool is_gradient() const
  {
    return m_type == Type::GradientPaint;
  }

  /**
   * @brief Checks if the paint is a texture.
   */
  inline bool is_texture() const
  {
    return m_type == Type::TexturePaint;
  }

  /**
   * @brief Returns the type of the parent component.
   */
  inline Type type() const
  {
    return m_type;
  }

  /**
   * @brief Returns the color of the paint.
   *
   * This method does not perform any type checking, type() should be called first.
   *
   * @return The color of the paint.
   */
  inline const vec4& color() const
  {
    return m_color;
  }

  /**
   * @brief Returns the id of the paint.
   *
   * This method does not perform any type checking, type() should be called first.
   *
   * @return The id of the paint.
   */
  inline const uuid& id() const
  {
    return m_id;
  }

 private:
  Type m_type;     // The type of the paint.

  union {
    vec4 m_color;  // The color of the paint.
    uuid m_id;     // The id of the swatch, gradient, or texture.
  };
};

/**
 * @brief Fill properties used to render a path.
 */
struct Fill {
  Paint paint;    // The paint used to fill the path.
  FillRule rule;  // The fill rule used to determine how self-intersecting paths are filled.

  /**
   * @brief Default constructor
   */
  Fill() : paint(vec4(0.0f, 0.0f, 0.0f, 1.0f)), rule(FillRule::NonZero) {}

  /**
   * @brief Complete color constructor
   */
  Fill(const vec4& color, FillRule rule) : paint(color), rule(rule) {}

  /**
   * @brief Complete paint constructor
   */
  Fill(const Paint& paint, FillRule rule) : paint(paint), rule(rule) {}
  Fill(const uuid paint_id, const Paint::Type paint_type, FillRule rule)
      : paint(paint_id, paint_type), rule(rule)
  {
  }

  /**
   * @brief Assignment operators.
   */
  Fill& operator=(const Fill& fill)
  {
    paint = fill.paint;
    rule = fill.rule;
    return *this;
  }

  /**
   * @brief Checks if the fill is visible.
   */
  inline bool visible() const
  {
    return paint.type() != Paint::Type::ColorPaint || paint.color().a > 0.0f;
  }
};

/**
 * @brief Stroke properties used to render a path.
 */
struct Stroke {
  Paint paint;         // The paint used to stroke the path.

  LineCap cap;         // The line cap used to determine how the ends of a stroke are drawn.
  LineJoin join;       // The line join used to determine how the corners of a stroke are drawn.

  double width;        // The width of the stroke.
  double miter_limit;  // The miter limit used to determine whether the join is mitered or beveled.

  /**
   * @brief Default constructor
   */
  Stroke()
      : paint(vec4(0.0f, 0.0f, 0.0f, 1.0f)),
        cap(LineCap::Butt),
        join(LineJoin::Miter),
        width(1.0),
        miter_limit(10.0)
  {
  }

  /**
   * @brief Complete color constructor
   */
  Stroke(const vec4& color, LineCap cap, LineJoin join, double width, double miter_limit)
      : paint(color), cap(cap), join(join), width(width), miter_limit(miter_limit)
  {
  }

  /**
   * @brief Complete paint constructor
   */
  Stroke(const Paint& paint, LineCap cap, LineJoin join, double width, double miter_limit)
      : paint(paint), cap(cap), join(join), width(width), miter_limit(miter_limit)
  {
  }
  Stroke(const uuid paint_id,
         const Paint::Type paint_type,
         LineCap cap,
         LineJoin join,
         double width,
         double miter_limit)
      : paint(paint_id, paint_type), cap(cap), join(join), width(width), miter_limit(miter_limit)
  {
  }

  /**
   * @brief Assignment operators.
   */
  Stroke& operator=(const Stroke& stroke)
  {
    paint = stroke.paint;
    cap = stroke.cap;
    join = stroke.join;
    width = stroke.width;
    miter_limit = stroke.miter_limit;
    return *this;
  }

  /**
   * @brief Checks if the stroke is visible.
   */
  inline bool visible() const
  {
    return paint.type() != Paint::Type::ColorPaint || paint.color().a > 0.0f;
  }
};

}  // namespace graphick::renderer
