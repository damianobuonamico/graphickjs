#pragma once

#if !defined(GK_CONF_DIST) && !defined(EMSCRIPTEN)
#define GK_USE_DEBUGGER 1
#define GK_DEBUGGER_INIT() Graphick::Utils::Debugger::init()
#define GK_DEBUGGER_SHUTDOWN() Graphick::Utils::Debugger::shutdown()
#define GK_DEBUGGER_RENDER() Graphick::Utils::Debugger::render()
#else
#define GK_USE_DEBUGGER 0
#define GK_DEBUGGER_INIT() ((void)0)
#define GK_DEBUGGER_SHUTDOWN() ((void)0)
#define GK_DEBUGGER_RENDER() ((void)0)
#endif

#ifdef GK_USE_DEBUGGER

#include "../renderer/geometry/path.h"

#include "../lib/stb/stb_truetype.h"

namespace Graphick::Utils {
  class Debugger {
  public:
    static void init();
    static void shutdown();

    static void render();
  private:
    Debugger() {}
    ~Debugger() = default;

    static inline Debugger* get() { return s_instance; }

    void create_glyphs();
  private:
    stbtt_fontinfo m_font_info;
    Renderer::Geometry::Path m_glyphs[96];
  private:
    static Debugger* s_instance;
  };

}

#endif
