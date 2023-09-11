#pragma once

#include "assert.h"

#include <stdio.h>
#include <sstream>
#include <ostream>
#include <chrono>
#include <bitset>
#include <unordered_map>

#if defined(GK_USE_OPTICK) && !defined(GK_CONF_DIST)
#include <optick.h>
#else
#define OPTICK_EVENT(...) ((void)0)
#define OPTICK_FRAME(...) ((void)0)
#endif

namespace Graphick::Utils {

  struct console {
  private:
    struct AverageTimer {
      std::chrono::steady_clock::time_point last_time;
      size_t duration = 0;
      size_t samples = 0;
    };
  public:
    template <typename T>
    static inline void log(T value) {
      std::stringstream stream;
      stream << value;

      printf("%s\n", stream.str().c_str());
    }

    template <typename T>
    static inline void log(const std::string& name, T value) {
      std::stringstream stream;
      stream << name << ": " << value;

      printf("%s\n", stream.str().c_str());
    }

    template <typename T>
    static inline void bitset(T value) {
      std::stringstream stream;
      stream << std::bitset<8 * sizeof(value)>(value);

      printf("%s\n", stream.str().c_str());
    }

    template <typename T>
    static inline void bitset(const std::string& name, T value) {
      std::stringstream stream;
      stream << name << ": " << std::bitset<8 * sizeof(value)>(value);

      printf("%s\n", stream.str().c_str());
    }

    template <typename T>
    static inline void error(T value) {
      std::stringstream stream;
      stream << value;

      printf("%s\n", stream.str().c_str());
    }

    template <typename T>
    static inline void error(const std::string& name, T value) {
      std::stringstream stream;
      stream << name << ": " << value;

      printf("%s\n", stream.str().c_str());
    }

    static inline void time_start() {
      m_last_time = std::chrono::high_resolution_clock::now();
    }

    static inline void time_end(const std::string& name = "Time Elapsed") {
      log(name, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_last_time).count());
    }

    static inline void average_start(const std::string& name) {
      auto it = m_timers.find(name);
      auto time = std::chrono::high_resolution_clock::now();

      if (it == m_timers.end()) {
        m_timers.insert({ name, { time } });
      } else {
        it->second.last_time = time;
      }
    }

    static inline void average_end(const std::string& name) {
      auto it = m_timers.find(name);
      auto time = std::chrono::high_resolution_clock::now();

      if (it == m_timers.end()) return;

      auto duration = time - it->second.last_time;

      it->second.duration = (it->second.duration * it->second.samples + std::chrono::duration_cast<std::chrono::microseconds>(duration).count()) / (it->second.samples + 1);
      it->second.samples++;

      log(name, it->second.duration);
    }
  private:
    static inline std::chrono::steady_clock::time_point m_last_time;
    static inline std::unordered_map<std::string, AverageTimer> m_timers;
  };

  struct ScopedTimer {
    ScopedTimer(const std::string& id) : id(id) {
      console::average_start(id);
    }

    ~ScopedTimer() {
      console::average_end(id);
    }

    std::string id;
  };

}

#ifdef GK_CONF_DIST
#define GK_AVERAGE ((void)0)
#else
#define GK_AVERAGE(name) Graphick::Utils::ScopedTimer __scoped_timer(name) 
#endif

namespace Graphick {
  using console = Utils::console;
}
