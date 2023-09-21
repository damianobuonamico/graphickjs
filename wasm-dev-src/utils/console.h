#pragma once

#include "assert.h"
#include "debugger.h"

#include <stdio.h>
#include <sstream>
#include <ostream>
#include <chrono>
#include <bitset>
#include <unordered_map>
#include <vector>
#include <numeric>

#if defined(GK_USE_OPTICK) && !defined(GK_CONF_DIST)
#include <optick.h>
#else
#define OPTICK_EVENT(...) ((void)0)
#define OPTICK_FRAME(...) ((void)0)
#endif

#define RECORDS_SIZE 100

namespace Graphick::Utils {

  struct console {
  private:
    struct AverageTimer {
      std::chrono::steady_clock::time_point last_time;
      size_t duration = 0;
      size_t samples = 0;
    };
    struct TotalTimer {
      std::chrono::steady_clock::time_point last_time;
      std::vector<size_t> records = std::vector<size_t>(RECORDS_SIZE);
      size_t index = 0;

      inline void start() {
        last_time = std::chrono::high_resolution_clock::now();
      }

      inline void end() {
        auto duration = std::chrono::high_resolution_clock::now() - last_time;
        records[index % RECORDS_SIZE] += std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
      }

      inline void next() {
        index++;
        records[index % RECORDS_SIZE] = 0;
      }

      inline size_t average() {
        return std::reduce(records.begin(), records.end()) / RECORDS_SIZE;
      }
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
      log(name, std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_last_time).count());
    }

    static inline void frame(const std::string& name) {
      GK_DEBUGGER_CLEAR();

      for (auto& [name, timer] : m_total_timers) {
        GK_DEBUGGER_LOG(name + ": " + std::to_string((float)timer.average() / 1000000.0f) + "ms");
        timer.next();
      }
    }

    static inline void total_start(const std::string& name) {
      auto it = m_total_timers.find(name);
      auto time = std::chrono::high_resolution_clock::now();

      if (it == m_total_timers.end()) {
        TotalTimer timer;
        timer.start();

        m_total_timers.insert({ name, timer });
      } else {
        it->second.start();
      }
    }

    static inline void total_end(const std::string& name) {
      auto it = m_total_timers.find(name);
      if (it == m_total_timers.end()) return;

      it->second.end();
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

      it->second.duration = (it->second.duration * it->second.samples + std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()) / (it->second.samples + 1);
      it->second.samples++;

      log(name, std::to_string((float)it->second.duration / 1000000.0f) + "ms");
    }
  private:
    static inline std::chrono::steady_clock::time_point m_last_time;
    static inline std::unordered_map<std::string, TotalTimer> m_total_timers;
    static inline std::unordered_map<std::string, AverageTimer> m_timers;
  };

  struct ScopedTimer {
    ScopedTimer(const std::string& id, const bool total = false) : id(id), total(total) {
      if (total) console::total_start(id);
      else console::average_start(id);
    }

    ~ScopedTimer() {
      if (total) console::total_end(id);
      else console::average_end(id);
    }

    std::string id;
    bool total;
  };

}

#ifdef GK_CONF_DIST
#define GK_TOTAL(...) ((void)0)
#define GK_AVERAGE(...) ((void)0)
#define GK_FRAME(...) ((void)0)
#else
#define GK_TOTAL(name) Graphick::Utils::ScopedTimer __scoped_timer(name, true)
#define GK_AVERAGE(name) Graphick::Utils::ScopedTimer __scoped_timer(name) 
#define GK_FRAME(name) Graphick::Utils::console::frame(name); GK_TOTAL("MainThread")
#endif

namespace Graphick {
  using console = Utils::console;
}
