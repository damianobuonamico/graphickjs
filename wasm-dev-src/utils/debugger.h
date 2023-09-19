#pragma once

#if !defined(GK_CONF_DIST) && !defined(EMSCRIPTEN)
#define GK_USE_DEBUGGER 1
#define GK_DEBUGGER_INIT() Graphick::Utils::Debugger::init()
#define GK_DEBUGGER_SHUTDOWN() Graphick::Utils::Debugger::shutdown()
#define GK_DEBUGGER_RENDER(...) Graphick::Utils::Debugger::render(__VA_ARGS__)
#define GK_DEBUGGER_CLEAR() Graphick::Utils::Debugger::clear()
#define GK_DEBUGGER_LOG(...) Graphick::Utils::Debugger::log(__VA_ARGS__)
#else
#define GK_USE_DEBUGGER 0
#define GK_DEBUGGER_INIT() ((void)0)
#define GK_DEBUGGER_SHUTDOWN() ((void)0)
#define GK_DEBUGGER_RENDER(...) ((void)0)
#define GK_DEBUGGER_CLEAR(...)
#define GK_DEBUGGER_LOG(...)
#endif

#if GK_USE_DEBUGGER

#include "../renderer/geometry/path.h"

#include "../lib/stb/stb_truetype.h"

#include <vector>
#include <string>

namespace Graphick::Utils {
  class Debugger {
  public:
    static void init();
    static void shutdown();

    static void clear();
    static void log(const std::string& text);

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
