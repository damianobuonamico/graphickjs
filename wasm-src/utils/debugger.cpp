#include "debugger.h"

#if GK_USE_DEBUGGER

#include "../renderer/renderer.h"
#include "../renderer/drawable.h"

#include "../math/mat2x3.h"
#include "../math/math.h"

#include <fstream>
#include <stdio.h>

namespace Graphick::Utils {

  Debugger* Debugger::s_instance = nullptr;

  void Debugger::init() {
    if (s_instance != nullptr) {
      return;
    }

    s_instance = new Debugger();
  }

  void Debugger::shutdown() {
    if (s_instance == nullptr) {
      return;
    }

    delete s_instance;
    s_instance = nullptr;
  }

  void Debugger::clear() {
    get()->m_messages.clear();
  }

  void Debugger::log(const std::string& text) {
    get()->m_messages.push_back(text);
  }

  void Debugger::draw(const Renderer::Geometry::Contour& contour, const mat2x3& transform, const vec4& color) {
    Renderer::Renderer::draw_outline(contour, transform, color);
  }

  void Debugger::draw(const Renderer::Geometry::Contour& contour, const mat2x3& transform) {
    Renderer::Renderer::draw_outline(contour, transform);
  }

  void Debugger::draw(const Renderer::Geometry::Contour& contour) {
    Renderer::Renderer::draw_outline(contour);
  }

  void Debugger::draw(const Renderer::Drawable& drawable, const mat2x3& transform, const vec4& color) {
    for (auto& contour : drawable.contours) {
      draw(contour, transform, color);
    }
  }

  void Debugger::draw(const Renderer::Drawable& drawable, const mat2x3& transform) {
    for (auto& contour : drawable.contours) {
      draw(contour, transform);
    }
  }

  void Debugger::draw(const Renderer::Drawable& drawable) {
    for (auto& contour : drawable.contours) {
      draw(contour);
    }
  }

  void Debugger::draw(const rect& rect) {
    Renderer::Geometry::Contour contour;

    contour.move_to(dvec2(rect.min));
    contour.line_to(dvec2{ rect.max.x, rect.min.y });
    contour.line_to(dvec2(rect.max));
    contour.line_to(dvec2{ rect.min.x, rect.max.y });
    contour.close();

    draw(contour);
  }

  void Debugger::render(const vec2 viewport_size) {
    // Renderer::Geometry::Path rect;
    // rect.move_to({ visible.max.x - 200.0f / zoom, visible.min.y });
    // rect.line_to({ visible.max.x, visible.min.y });
    // rect.line_to({ visible.max.x, visible.min.y + 200.0f / zoom });
    // rect.line_to({ visible.max.x - 200.0f / zoom, visible.min.y + 200.0f / zoom });
    // rect.close();

    // Renderer::Renderer::draw(get()->m_glyphs['a' - 32], 0.0001f, vec2{ visible.max.x - 190.0f / zoom, visible.min.y + 20.0f / zoom }, vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    // Renderer::Renderer::draw(rect, 0.0001f, vec2{ 0.0f, 0.0f }, vec4{ 0.0f, 0.0f, 0.0f, 0.5f });

    size_t messages_count = get()->m_messages.size();

    Renderer::Renderer::debug_rect({ {viewport_size.x - 350.0f, 0 }, { viewport_size.x, messages_count * 20.0f + 8.0f } }, vec4{ 0.0f, 0.0f, 0.0f, 0.7f });

    for (size_t i = 0; i < messages_count; i++) {
      Renderer::Renderer::debug_text(get()->m_messages[i], { viewport_size.x - 340.0f, 18.0f + i * 20.0f }, vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    }

    // Math::rect bounding_rect = get()->m_glyphs['a' - 32].bounding_rect();
    // std::vector<Math::rect> lines = Math::lines_from_rect(bounding_rect);
    // Renderer::Geometry::Path rect;
    // rect.move_to(lines[0].min);

    // for (auto& line : lines) {
    //   rect.line_to(line.max);
    // }

    // Renderer::Renderer::draw_outline(0, rect, vec2{ 0.0f, 0.0f });
  }

  // void Debugger::create_glyphs() {
  //   stbtt_vertex* vertices;
  //   size_t num_vertices;

  //   float scale = stbtt_ScaleForPixelHeight(&m_font_info, 24);

  //   for (int i = 0; i < 96; i++) {
  //     num_vertices = stbtt_GetCodepointShape(&m_font_info, i + 32, &vertices);

  //     Renderer::Geometry::Path& path = m_glyphs[i];
  //     bool is_first_move = true;
  //     bool should_break = false;

  //     for (int j = 0; j < num_vertices; j++) {
  //       stbtt_vertex& vertex = vertices[j];

  //       switch (vertex.type) {
  //       case STBTT_vline:
  //         path.line_to(scale * vec2{ (float)vertex.x, -(float)vertex.y });
  //         break;
  //       case STBTT_vcurve:
  //         path.quadratic_to(scale * vec2{ (float)vertex.cx, -(float)vertex.cy }, scale * vec2{ (float)vertex.x, -(float)vertex.y });
  //         break;
  //       case STBTT_vcubic:
  //         path.cubic_to(scale * vec2{ (float)vertex.cx, -(float)vertex.cy }, scale * vec2{ (float)vertex.cx1, -(float)vertex.cy1 }, scale * vec2{ (float)vertex.x, -(float)vertex.y });
  //         break;
  //       default:
  //       case STBTT_vmove:
  //         if (is_first_move) {
  //           path.move_to(scale * vec2{ (float)vertex.x, -(float)vertex.y });
  //           is_first_move = false;
  //         } else {
  //           should_break = true;
  //         }
  //         break;
  //       }

  //       if (should_break) break;
  //     }

  //     delete[] vertices;
  //   }

  // }

}

#endif
