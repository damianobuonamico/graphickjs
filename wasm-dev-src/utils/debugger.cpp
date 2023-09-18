#include "debugger.h"

#ifdef GK_USE_DEBUGGER

#include "console.h"

#include "../renderer/renderer.h"

#include "../math/math.h"

#include <fstream>
#include <stdio.h>

namespace Graphick::Utils {

  Debugger* Debugger::s_instance = nullptr;

  void Debugger::init() {
    if (s_instance != nullptr) {
      console::error("Debugger already initialized, call shutdown() before reinitializing!");
      return;
    }

    s_instance = new Debugger();

    /* load font file */
    long size;
    unsigned char* fontBuffer;

    FILE* fontFile = fopen("res\\fonts\\times.ttf", "rb");
    fseek(fontFile, 0, SEEK_END);
    size = ftell(fontFile); /* how long is the file ? */
    fseek(fontFile, 0, SEEK_SET); /* reset */

    fontBuffer = new unsigned char[size];

    fread(fontBuffer, size, 1, fontFile);
    fclose(fontFile);

    if (!stbtt_InitFont(&get()->m_font_info, fontBuffer, 0)) {
      console::error("Failed to initialize font info!");
    }

    get()->create_glyphs();
  }

  void Debugger::shutdown() {
    if (s_instance == nullptr) {
      console::error("Renderer already shutdown, call init() before shutting down!");
      return;
    }

    delete s_instance;
  }

  void Debugger::render() {
    Renderer::Renderer::draw(get()->m_glyphs['a' - 32], 0.1f, vec2{ 0.0f, 0.0f }, vec4{ 0.0f, 0.0f, 0.0f, 1.0f });

    // Math::rect bounding_rect = get()->m_glyphs['a' - 32].bounding_rect();
    // std::vector<Math::rect> lines = Math::lines_from_rect(bounding_rect);
    // Renderer::Geometry::Path rect;
    // rect.move_to(lines[0].min);

    // for (auto& line : lines) {
    //   rect.line_to(line.max);
    // }

    // Renderer::Renderer::draw_outline(0, rect, vec2{ 0.0f, 0.0f });
  }

  void Debugger::create_glyphs() {
    stbtt_vertex* vertices;
    size_t num_vertices;

    float scale = stbtt_ScaleForPixelHeight(&m_font_info, 24);

    for (int i = 0; i < 96; i++) {
      num_vertices = stbtt_GetCodepointShape(&m_font_info, i + 32, &vertices);

      Renderer::Geometry::Path& path = m_glyphs[i];
      bool is_first_move = true;
      bool should_break = false;

      for (int j = 0; j < num_vertices; j++) {
        stbtt_vertex& vertex = vertices[j];

        switch (vertex.type) {
        case STBTT_vline:
          path.line_to(scale * vec2{ (float)vertex.x, -(float)vertex.y });
          break;
        case STBTT_vcurve:
          path.quadratic_to(scale * vec2{ (float)vertex.cx, -(float)vertex.cy }, scale * vec2{ (float)vertex.x, -(float)vertex.y });
          break;
        case STBTT_vcubic:
          path.cubic_to(scale * vec2{ (float)vertex.cx, -(float)vertex.cy }, scale * vec2{ (float)vertex.cx1, -(float)vertex.cy1 }, scale * vec2{ (float)vertex.x, -(float)vertex.y });
          break;
        default:
        case STBTT_vmove:
          if (is_first_move) {
            path.move_to(scale * vec2{ (float)vertex.x, -(float)vertex.y });
            is_first_move = false;
          } else {
            should_break = true;
          }
          break;
        }

        if (should_break) break;
      }

      delete[] vertices;
    }

  }

}

#endif
