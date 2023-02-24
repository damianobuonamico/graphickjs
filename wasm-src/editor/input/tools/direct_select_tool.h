#pragma once

#include "../tool.h"
#include "../../scene/entities/element_entity.h"
#include "../../scene/entities/selection_rect_entity.h"

class DirectSelectTool: public Tool {
public:
  virtual void on_pointer_down() override;
  virtual void on_pointer_move() override;
  virtual void on_pointer_up(bool abort = false) override;

  virtual void tessellate_overlays_outline(const vec4& color, float zoom, Geometry& geo) const override;
  virtual void render_overlays(float zoom) const override;
private:
  DirectSelectTool();

  void translate_selected();
  void shift_select_element(Entity* entity);

  void on_none_pointer_down();
  void on_duplicate_pointer_down();
  void on_entity_pointer_down();
  void on_element_pointer_down();
  void on_bezier_pointer_down();
  void on_vertex_pointer_down();
  void on_handle_pointer_down();

  void on_none_pointer_move();
  void on_duplicate_pointer_move();
  void on_entity_pointer_move();
  void on_element_pointer_move();
  void on_bezier_pointer_move();
  void on_vertex_pointer_move();
  void on_handle_pointer_move();

  void on_none_pointer_up();
  void on_duplicate_pointer_up();
  void on_entity_pointer_up();
  void on_element_pointer_up();
  void on_bezier_pointer_up();
  void on_vertex_pointer_up();
  void on_handle_pointer_up();
private:
  enum Mode {
    ModeNone = 0,
    ModeDuplicate,
    ModeElement,
    ModeVertex,
    ModeHandle,
    ModeBezier,
    ModeEntity
  };
private:
  bool m_dragging_occurred = false;
  bool m_is_entity_added_to_selection = false;
  bool m_should_evaluate_selection = false;
  vec2 m_last_bezier_point{};
  vec2 m_last_bezier_p1{};
  vec2 m_last_bezier_p2{};
  BezierEntity::BezierPointDistance m_closest{};

  Mode m_mode = ModeNone;

  Entity* m_entity = nullptr;
  ElementEntity* m_element = nullptr;
  BezierEntity* m_bezier = nullptr;
  VertexEntity* m_vertex = nullptr;
  HandleEntity* m_handle = nullptr;

  SelectionRectEntity m_selection_rect{ true };
private:
  friend class ToolState;
};
