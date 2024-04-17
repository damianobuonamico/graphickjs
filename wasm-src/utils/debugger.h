#pragma once

#if !defined(GK_CONF_DIST) && !defined(EMSCRIPTEN)
#define GK_USE_DEBUGGER 1
#define GK_DEBUGGER_OVERLAYS 1

#define GK_DEBUGGER_INIT() graphick::Utils::Debugger::init()
#define GK_DEBUGGER_SHUTDOWN() graphick::Utils::Debugger::shutdown()
#define GK_DEBUGGER_RENDER(...) graphick::Utils::Debugger::render(__VA_ARGS__)
#define GK_DEBUGGER_CLEAR() graphick::Utils::Debugger::clear()
#define GK_DEBUGGER_LOG(...) graphick::Utils::Debugger::log(__VA_ARGS__)

#if GK_DEBUGGER_OVERLAYS
#define GK_DEBUGGER_DRAW(...) graphick::Utils::Debugger::draw(__VA_ARGS__)
#else
#define GK_DEBUGGER_DRAW(...) ((void)0)
#endif
#else
#define GK_USE_DEBUGGER 0

#define GK_DEBUGGER_INIT() ((void)0)
#define GK_DEBUGGER_SHUTDOWN() ((void)0)
#define GK_DEBUGGER_RENDER(...) ((void)0)
#define GK_DEBUGGER_CLEAR(...) ((void)0)
#define GK_DEBUGGER_LOG(...) ((void)0)

#define GK_DEBUGGER_DRAW(...) ((void)0)
#endif

#if GK_USE_DEBUGGER

#include "../math/vec4.h"
#include "../math/vec2.h"
#include "../math/mat2x3.h"
#include "../math/rect.h"

#include "../lib/stb/stb_truetype.h"

#include <vector>
#include <string>

namespace graphick::renderer {
  struct Drawable;
}

namespace graphick::renderer::geometry {
  class Contour;
}

namespace graphick::utils {
  class Debugger {
  public:
    static void init();
    static void shutdown();

    static void clear();
    static void log(const std::string& text);

    static void draw(const Renderer::Geometry::Contour& contour, const mat2x3& transform, const vec4& color);
    static void draw(const Renderer::Geometry::Contour& contour, const mat2x3& transform);
    static void draw(const Renderer::Geometry::Contour& contour);

    static void draw(const Renderer::Drawable& drawable, const mat2x3& transform, const vec4& color);
    static void draw(const Renderer::Drawable& drawable, const mat2x3& transform);
    static void draw(const Renderer::Drawable& drawable);

    static void draw(const rect& rect);

    static void render(const vec2 viewport_size);
  private:
    Debugger() {}
    ~Debugger() = default;

    static inline Debugger* get() { return s_instance; }
  private:
    std::vector<std::string> m_messages;
  private:
    static Debugger* s_instance;
  };

}

#endif
