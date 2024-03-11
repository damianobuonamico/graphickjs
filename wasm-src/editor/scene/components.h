/**
 * @file components.h
 * @brief Components to be added to entities.
 *
 * @todo doc
 */

#pragma once

#include "../../renderer/geometry/path.h"
#include "../../renderer/properties.h"

#include "../../math/vec4.h"
#include "../../math/rect.h"
#include "../../math/mat2x3.h"

#include "../../utils/uuid.h"

#include <vector>
#include <string>

namespace Graphick::Editor {

  class Entity;

  /**
   * @brief Base component wrapper struct.
   *
   * This struct is a wrapper around the actual component data, to allow for manipulation and history tracking.
   * Holds a pointer to the entity it belongs to.
   *
   * @struct ComponentWrapper
   */
  struct ComponentWrapper {
  public:
    /**
     * @brief Constructor.
     */
    ComponentWrapper(const Entity* entity) : m_entity(entity) {}

    /**
     * @brief Encodes the component in binary format.
     *
     * @param data The encoded data to append the component to.
     * @param optimize Whether to skip encoding if the component is in a default state. Default is true.
     * @return A reference to the encoded data.
     */
    virtual io::EncodedData& encode(io::EncodedData& data, const bool optimize = false) const = 0;
  protected:
    /**
     * @brief Modifies the underlying data of the component.
     *
     * @param decoder A diff of the modified component's data.
     */
    virtual void modify(io::DataDecoder& decoder) = 0;
  protected:
    const Entity* m_entity;    /* A pointer to the entity this component belongs to. */
  };

  /**
   * @brief IDComponent data.
   *
   * This struct should not be used directly, use the IDComponent wrapper instead.
   *
   * @struct IDComponentData
   */
  struct IDComponentData {
    uuid id;    /* The id of the entity. */

    IDComponentData();
    IDComponentData(const uuid id);
    IDComponentData(io::DataDecoder& decoder);
  };

  /**
   * @brief IDComponent wrapper.
   *
   * Once an IDComponent is created, it cannot be modified.
   *
   * @struct IDComponent
   */
  struct IDComponent : public ComponentWrapper {
  public:
    static constexpr uint8_t component_id = 0;    /* The component id. */

    using Data = IDComponentData;                 /* The component underlying data type. */
  public:
    /**
     * @brief Constructor.
     */
    IDComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

    /**
     * @brief Conversion operator to uuid.
     */
    inline operator uuid() const { return m_data->id; }

    /**
     * @brief Returns the id of the entity.
     *
     * @return The id of the entity.
     */
    inline uuid id() const { return m_data->id; }

    /**
     * @brief Encodes the component in binary format.
     *
     * @param data The encoded data to append the component to.
     * @param optimize Whether to skip encoding if the component is in a default state. Default is true.
     * @return A reference to the encoded data.
     */
    io::EncodedData& encode(io::EncodedData& data, const bool optimize = false) const override;
  private:
    /**
     * @brief An IDComponent cannot be modified.
     *
     * @param decoder A diff of the modified component's data.
     */
    inline void modify(io::DataDecoder& decoder) override {};
  private:
    Data* m_data;    /* The actual component data. */
  };

  /**
   * @brief TagComponent data.
   *
   * This struct should not be used directly, use the TagComponent wrapper instead.
   *
   * @struct TagComponentData
   */
  struct TagComponentData {
    std::string tag = "";    /* The tag of the entity. */

    TagComponentData() = default;
    TagComponentData(const std::string& tag);
    TagComponentData(io::DataDecoder& decoder);
  };

  /**
   * @brief TagComponent wrapper.
   *
   * A tag is the display name of an entity, it is only used to identify the entity in the editor.
   *
   * @struct TagComponent
   */
  struct TagComponent : public ComponentWrapper {
  public:
    static constexpr uint8_t component_id = 1;    /* The component id. */

    using Data = TagComponentData;                /* The component underlying data type. */
  public:
    /**
     * @brief Constructor.
     */
    TagComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

    /**
     * @brief Conversion operator to std::string&.
     */
    inline operator const std::string& () const { return m_data->tag; }

    /**
     * @brief Returns the tag of the entity.
     *
     * @return The tag of the entity.
     */
    inline const std::string& tag() const { return m_data->tag; }

    /**
     * @brief Encodes the component in binary format.
     *
     * @param data The encoded data to append the component to.
     * @param optimize Whether to skip encoding if the component is in a default state. Default is true.
     * @return A reference to the encoded data.
     */
    io::EncodedData& encode(io::EncodedData& data, const bool optimize = false) const override;
  private:
    /**
     * @brief Modifies the underlying data of the component.
     *
     * @param decoder A diff of the modified component's data.
     */
    void modify(io::DataDecoder& decoder) override;
  private:
    Data* m_data;    /* The actual component data. */
  private:
    friend class Entity;
  };

  /**
   * @brief CategoryComponent data.
   *
   * This struct should not be used directly, use the CategoryComponent wrapper instead.
   *
   * @struct CategoryComponentData
   */
  struct CategoryComponentData {
    enum Category {
      None = 0,               /* The entity is not in any category. */
      Selectable = 1 << 0,    /* The entity can be selected. */
    };

    int category = None;      /* A bitfield of the category flags. */

    CategoryComponentData() = default;
    CategoryComponentData(const int category);
    CategoryComponentData(io::DataDecoder& decoder);
  };

  /**
   * @brief CategoryComponent wrapper.
   *
   * A category is a set of flags that define the behavior of an entity in the editor.
   *
   * @struct CategoryComponent
   */
  struct CategoryComponent : public ComponentWrapper {
  public:
    static constexpr uint8_t component_id = 2;    /* The component id. */

    using Data = CategoryComponentData;           /* The component underlying data type. */
    using Category = Data::Category;              /* The category enum. */
  public:
    /**
     * @brief Constructor.
     */
    CategoryComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

    /**
     * @brief Conversion operator to int.
     */
    inline operator int() const { return m_data->category; }

    /**
     * @brief Returns the category flags of the entity.
     *
     * @return The category of the entity.
     */
    inline int category() const { return m_data->category; }

    /**
     * @brief Checks if the entity is in the specified category.
     *
     * @param category The category to check.
     * @return true if the entity is in the category, false otherwise.
     */
    inline bool is_in_category(Category category) const { return m_data->category & category; }

    /**
     * @brief Encodes the component in binary format.
     *
     * @param data The encoded data to append the component to.
     * @param optimize Whether to skip encoding if the component is in a default state. Default is true.
     * @return A reference to the encoded data.
     */
    io::EncodedData& encode(io::EncodedData& data, const bool optimize = false) const override;
  private:
    /**
     * @brief Modifies the underlying data of the component.
     *
     * @param decoder A diff of the modified component's data.
     */
    void modify(io::DataDecoder& decoder) override;
  private:
    Data* m_data;    /* The actual component data. */
  private:
    friend class Entity;
  };

  /**
   * @brief PathComponent data.
   *
   * This struct should not be used directly, use the PathComponent wrapper instead.
   *
   * @struct PathComponentData
   */
  struct PathComponentData {
    Renderer::Geometry::Path path;    /* The path data. */

    PathComponentData();
    PathComponentData(const Renderer::Geometry::Path& path);
    PathComponentData(io::DataDecoder& decoder);
  };

  /**
   * @brief PathComponent wrapper.
   *
   * A path is a set of points and commands that define the shape of an element entity.
   *
   * @struct PathComponent
   */
  struct PathComponent : public ComponentWrapper {
  public:
    static constexpr uint8_t component_id = 3;    /* The component id. */

    using Data = PathComponentData;               /* The component underlying data type. */
  public:
    /**
     * @brief Constructor.
     */
    PathComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

    /**
     * @brief Conversion operator to Renderer::Geometry::Path&.
     */
    inline operator const Renderer::Geometry::Path& () const { return m_data->path; }

    /**
     * @brief Returns the path data of the entity.
     *
     * @return The path data of the entity.
     */
    inline const Renderer::Geometry::Path& data() const { return m_data->path; }

    // TEMP
    inline Renderer::Geometry::Path& TEMP_path() { return m_data->path; }
    inline const Renderer::Geometry::Path& TEMP_path() const { return m_data->path; }

    /**
     * @brief Moves the path cursor to the given point.
     *
     * @param p0 The point to move the cursor to.
     * @return The index of the newly added control point.
     */
    size_t move_to(const vec2 p0);

    /**
     * @brief Adds a line segment to the path.
     *
     * @param p1 The point to add to the path.
     * @param reverse Whether to add the point to the beginning of the path, instead of the end. Default is false.
     * @return The index of the newly added control point.
     */
    size_t line_to(const vec2 p1, const bool reverse = false);

    /**
     * @brief Adds a cubic bezier curve to the path.
     *
     * @param p1 The first control point of the curve.
     * @param p2 The second control point of the curve.
     * @param p2 The point to add to the path.
     * @param reverse Whether to add the point to the beginning of the path, instead of the end. Default is false.
     * @return The index of the newly added control point.
     */
    size_t cubic_to(const vec2 p1, const vec2 p2, const vec2 p3, const bool reverse = false);

    /**
     * @brief Closes the path by adding a segment to the first point in the path.
     *
     * If incoming or outgoing handles are present, the new segment will be a cubic bezier curve.
     *
     * @return The index of the newly added control point.
     */
    size_t close();

    /**
     * @brief Translates a control point in the path by a given delta.
     *
     * @param point_index The index of the point to translate.
     * @param delta The translation delta.
     */
    void translate(const size_t point_index, const vec2 delta);

    /**
     * @brief Converts the given command to a line command.
     *
     * @param command_index The index of the command to convert.
     * @return The number of control points removed from the path.
     */
    size_t to_line(const size_t command_index);

    /**
     * @brief Converts the given command to a cubic command.
     *
     * @param command_index The index of the command to convert.
     * @param reference_point The control point to return the updated index of
     * @return The updated index of the reference point.
     */
    size_t to_cubic(const size_t command_index, const size_t reference_point = 0);

    /**
     * @brief Splits the segment at the given index at the given t value.
     *
     * @param segment_index The index of the segment to split.
     * @param t The t value to split the segment at.
     * @return The index of the newly added vertex.
     */
    size_t split(const size_t segment_index, const float t);

    /**
     * @brief Removes the ith control point from the path.
     *
     * @param point_index The index of the control point to remove.
     * @param keep_shape Whether to keep the shape of the path after removing the control point. Default is false.
     */
    void remove(const size_t point_index, const bool keep_shape = false);

    /**
     * @brief Encodes the component in binary format.
     *
     * @param data The encoded data to append the component to.
     * @param optimize Whether to skip encoding if the component is in a default state. Default is true.
     * @return A reference to the encoded data.
     */
    io::EncodedData& encode(io::EncodedData& data, const bool optimize = false) const override;
  private:
    /**
     * @brief Modifies the underlying data of the component.
     *
     * @param decoder A diff of the modified component's data.
     */
    void modify(io::DataDecoder& decoder) override;
  private:
    enum class PathModifyType {
      LoadData = 0,
      ModifyPoint = 1 << 0
    };
  private:
    Data* m_data;    /* The actual component data. */
  private:
    friend class Entity;
  };

  /**
   * @brief TransformComponent data.
   *
   * This struct should not be used directly, use the TransformComponent wrapper instead.
   *
   * @struct TransformComponentData
   */
  struct TransformComponentData {
    mat2x3 matrix = mat2x3{ 1.0f };    /* The transformation matrix. */

    TransformComponentData() = default;
    TransformComponentData(const mat2x3& matrix);
    TransformComponentData(io::DataDecoder& decoder);
  };

  /**
   * @brief TransformComponent wrapper.
   *
   * A transform is a 2x3 matrix used for translating, rotating and scaling an entity.
   * The bounding rect of an entity can be accessed through this component.
   *
   * @struct TransformComponent
   */
  struct TransformComponent : public ComponentWrapper {
  public:
    static constexpr uint8_t component_id = 4;    /* The component id. */

    using Data = TransformComponentData;          /* The component underlying data type. */
  public:
    /**
     * @brief Constructor.
     */
    TransformComponent(const Entity* entity, Data* data, const PathComponentData* path_ptr = nullptr)
      : ComponentWrapper(entity), m_data(data), m_path_ptr(path_ptr) {}

    /**
     * @brief Conversion operator to mat2x3.
     */
    inline operator mat2x3() const { return m_data->matrix; }

    /**
     * @brief Returns the transformation matrix of the entity.
     *
     * @return A mat2x3 representing the transformation matrix of the entity.
     */
    inline mat2x3 matrix() const { return m_data->matrix; }

    /**
     * @brief Returns the inverse of the transformation matrix.
     *
     * @return A mat2x3 representing the inverse of the transformation matrix.
     */
    mat2x3 inverse() const;

    /**
     * @brief Calculates the bounding rectangle of the entity.
     *
     * @return The bounding rectangle of the entity.
     */
    rect bounding_rect() const;

    /**
     * @brief Calculates the approximate bounding rectangle of the entity.
     *
     * This function is faster than the bounding_rect function, but the result is not as accurate.
     *
     * @return The approximate bounding rectangle of the entity.
     */
    rect approx_bounding_rect() const;

    /**
     * @brief Transforms a point using the transformation matrix.
     *
     * @param point The point to transform.
     * @return The transformed point.
     */
    inline vec2 transform(const vec2 point) const { return m_data->matrix * point; }

    /**
     * @brief Reverts a point using the inverse of the transformation matrix.
     *
     * @param point The point to revert.
     * @return The reverted point.
     */
    vec2 revert(const vec2 point) const;

    /**
     * @brief Translates the entity by a given delta.
     *
     * @param delta The translation delta.
     */
    void translate(const vec2 delta);

    /**
     * @brief Scales the entity by a given delta.
     *
     * @param delta The scale delta.
     */
    void scale(const vec2 delta);

    /**
     * @brief Rotates the entity by a given delta.
     *
     * @param delta The rotation delta.
     */
    void rotate(const float delta);

    /**
     * @brief Sets the transformation matrix of the entity.
     *
     * @param matrix The new transformation matrix.
     */
    void set(const mat2x3 matrix);

    /**
     * @brief Encodes the component in binary format.
     *
     * @param data The encoded data to append the component to.
     * @param optimize Whether to skip encoding if the component is in a default state. Default is true.
     * @return A reference to the encoded data.
     */
    io::EncodedData& encode(io::EncodedData& data, const bool optimize = false) const override;
  private:
    /**
     * @brief Modifies the underlying data of the component.
     *
     * @param decoder A diff of the modified component's data.
     */
    void modify(io::DataDecoder& decoder);
  private:
    Data* m_data;                           /* The actual component data. */

    const PathComponentData* m_path_ptr;    /* A pointer to the path component of the entity, can be nullptr if the entity is not an element. */
  private:
    friend class Entity;
  };

  /**
   * @brief StrokeComponent data.
   *
   * This struct should not be used directly, use the StrokeComponent wrapper instead.
   *
   * @struct StrokeComponentData
   */
  struct StrokeComponentData {
    vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };                /* The stroke color. */

    Renderer::LineCap cap = Renderer::LineCap::Butt;        /* The line cap. */
    Renderer::LineJoin join = Renderer::LineJoin::Miter;    /* The line join. */

    float width = 1.0f;                                     /* The line width. */
    float miter_limit = 10.0f;                              /* The miter limit, only used if join is set to miter. */

    bool visible = true;                                    /* Whether or not to display the stroke. */

    StrokeComponentData() = default;
    StrokeComponentData(const vec4& color);
    StrokeComponentData(io::DataDecoder& decoder);
  };

  /**
   * @brief StrokeComponent wrapper.
   *
   * A stroke is a collection of proprerties used for rendering.
   *
   * @struct StrokeComponent
   */
  struct StrokeComponent : public ComponentWrapper {
  public:
    static constexpr uint8_t component_id = 5;    /* The component id. */

    using Data = StrokeComponentData;             /* The component underlying data type. */
  public:
    /**
     * @brief Constructor.
     */
    StrokeComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

    // TEMP
    const Data& stroke_TEMP() const { return *m_data; }

    /**
     * @brief Encodes the component in binary format.
     *
     * @param data The encoded data to append the component to.
     * @param optimize Whether to skip encoding if the component is in a default state. Default is true.
     * @return A reference to the encoded data.
     */
    io::EncodedData& encode(io::EncodedData& data, const bool optimize = false) const override;
  private:
    /**
     * @brief Modifies the underlying data of the component.
     *
     * @param decoder A diff of the modified component's data.
     */
    void modify(io::DataDecoder& decoder) override;
  private:
    Data* m_data;    /* The actual component data. */
  private:
    friend class Entity;
  };

  /**
   * @brief FillComponent data.
   *
   * This struct should not be used directly, use the FillComponent wrapper instead.
   *
   * @struct FillComponentData
   */
  struct FillComponentData {
    vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };                  /* The stroke color. */

    Renderer::FillRule rule = Renderer::FillRule::NonZero;    /* The line cap. */

    bool visible = true;                                      /* Whether or not to display the stroke. */

    FillComponentData() = default;
    FillComponentData(const vec4& color);
    FillComponentData(io::DataDecoder& decoder);
  };

  /**
   * @brief FillComponent wrapper.
   *
   * A stroke is a collection of proprerties used for rendering.
   *
   * @struct FillComponent
   */
  struct FillComponent : public ComponentWrapper {
  public:
    static constexpr uint8_t component_id = 6;    /* The component id. */

    using Data = FillComponentData;               /* The component underlying data type. */
  public:
    /**
     * @brief Constructor.
     */
    FillComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

    // TEMP
    const Data& fill_TEMP() const { return *m_data; }

    /**
     * @brief Encodes the component in binary format.
     *
     * @param data The encoded data to append the component to.
     * @param optimize Whether to skip encoding if the component is in a default state. Default is true.
     * @return A reference to the encoded data.
     */
    io::EncodedData& encode(io::EncodedData& data, const bool optimize = false) const override;
  private:
    /**
     * @brief Modifies the underlying data of the component.
     *
     * @param decoder A diff of the modified component's data.
     */
    void modify(io::DataDecoder& decoder) override;
  private:
    Data* m_data;    /* The actual component data. */
  private:
    friend class Entity;
  };

}
