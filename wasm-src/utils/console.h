/**
 * @file console.h
 * @brief The file contains the definition of the console utility.
 *
 * @todo refactor timers and debugger
 */

#pragma once

#include "assert.h"
#include "debugger.h"

#include "../math/vec2.h"
#include "../math/vec3.h"
#include "../math/vec4.h"

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

  /**
   * @brief The class that represents the console utility.
   *
   * The console utility is responsible for logging messages and measuring time.
   *
   * @struct console
   */
  struct console {
  private:
    /**
     * @brief The structure that represents a timer used to measure the average time of a task.
     *
     * @struct AverageTimer
     */
    struct AverageTimer {
      std::chrono::steady_clock::time_point last_time;    /* The last time the timer was started. */
      size_t duration = 0;                                /* The total duration of the task. */
      size_t samples = 0;                                 /* The number of samples. */
    };

    /**
     * @brief The structure that represents a timer used to measure the total time of a task during a frame.
     *
     * @struct TotalTimer
     */
    struct TotalTimer {
      std::chrono::steady_clock::time_point last_time;                    /* The last time the timer was started. */
      std::vector<size_t> records = std::vector<size_t>(RECORDS_SIZE);    /* The records of the task. */
      size_t index = 0;                                                   /* The index of the current record. */

      /**
       * @brief Starts the timer.
       */
      inline void start() {
        last_time = std::chrono::high_resolution_clock::now();
      }

      /**
       * @brief Ends the timer.
       */
      inline void end() {
        auto duration = std::chrono::high_resolution_clock::now() - last_time;
        records[index % RECORDS_SIZE] += std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
      }

      /**
       * @brief Moves to the next record.
       */
      inline void next() {
        index++;
        records[index % RECORDS_SIZE] = 0;
      }

      /**
       * @brief Returns the average time of the task.
       *
       * @return The average time of the task.
       */
      inline size_t average() {
        return std::reduce(records.begin(), records.end()) / RECORDS_SIZE;
      }
    };
  public:
    /**
     * @brief Logs a message to the console.
     *
     * @param value The message to log.
     */
    template <typename T>
    static inline void log(T value) {
      std::stringstream stream;
      stream << value;

      printf("%s\n", stream.str().c_str());
    }

    /**
     * @brief Logs a message to the console.
     *
     * @param name The name of the message.
     * @param value The message to log.
     */
    template <typename T>
    static inline void log(const std::string& name, T value) {
      std::stringstream stream;
      stream << name << ": " << value;

      printf("%s\n", stream.str().c_str());
    }

    /**
     * @brief Logs a bitset to the console.
     *
     * @param value The value to bitset.
     */
    template <typename T>
    static inline void bitset(T value) {
      std::stringstream stream;
      stream << std::bitset<8 * sizeof(value)>(value);

      printf("%s\n", stream.str().c_str());
    }

    /**
     * @brief Logs a bitset to the console.
     *
     * @param name The name of the bitset.
     * @param value The value to bitset.
     */
    template <typename T>
    static inline void bitset(const std::string& name, T value) {
      std::stringstream stream;
      stream << name << ": " << std::bitset<8 * sizeof(value)>(value);

      printf("%s\n", stream.str().c_str());
    }

    /**
     * @brief Logs an error message to the console.
     *
     * @param value The error message to log.
     */
    template <typename T>
    static inline void error(T value) {
      std::stringstream stream;
      stream << value;

      printf("%s\n", stream.str().c_str());
    }

    /**
     * @brief Logs an error message to the console.
     *
     * @param name The name of the error message.
     * @param value The error message to log.
     */
    template <typename T>
    static inline void error(const std::string& name, T value) {
      std::stringstream stream;
      stream << name << ": " << value;

      printf("%s\n", stream.str().c_str());
    }

    /**
     * @brief Starts a timer.
     *
     * @param name The name of the time elapsed message.
     */
    static inline void time_start() {
      m_last_time = std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief Logs a time elapsed message to the console.
     *
     * To use this method, the time_start() method must be called first.
     *
     * @param name The name of the time elapsed message.
     */
    static inline void time_end(const std::string& name = "Time Elapsed") {
      log(name, std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_last_time).count());
    }

    /**
     * @brief Starts a new frame and logs the last frame's times.
     *
     * @param name The name of the frame.
     */
    static inline void frame(const std::string& name) {
      GK_DEBUGGER_CLEAR();

      for (auto& [name, timer] : m_total_timers) {
        console::log("Total " + name + ": " + std::to_string((float)timer.average() / 1000000.0f) + "ms");
        GK_DEBUGGER_LOG(name + ": " + std::to_string((float)timer.average() / 1000000.0f) + "ms");
        timer.next();
      }
    }

    /**
     * @brief Starts a total timer.
     *
     * @param name The name of the timer.
     */
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

    /**
     * @brief Ends a total timer.
     *
     * @param name The name of the timer.
     */
    static inline void total_end(const std::string& name) {
      auto it = m_total_timers.find(name);
      if (it == m_total_timers.end()) return;

      it->second.end();
    }

    /**
     * @brief Starts an average timer.
     *
     * @param name The name of the timer.
     */
    static inline void average_start(const std::string& name) {
      auto it = m_timers.find(name);
      auto time = std::chrono::high_resolution_clock::now();

      if (it == m_timers.end()) {
        m_timers.insert({ name, { time } });
      } else {
        it->second.last_time = time;
      }
    }

    /**
     * @brief Ends an average timer.
     *
     * @param name The name of the timer.
     */
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
    static inline std::chrono::steady_clock::time_point m_last_time;             /* The last time the timer was started. */
    static inline std::unordered_map<std::string, TotalTimer> m_total_timers;    /* The total timers. */
    static inline std::unordered_map<std::string, AverageTimer> m_timers;        /* The average timers. */
  };

  /**
   * @brief The structure that represents a scoped timer.
   *
   * It is used by the GK_TOTAL and GK_AVERAGE macros.
   *
   * @struct ScopedTimer
   */
  struct ScopedTimer {
    std::string id;    /* The name of the timer. */
    bool total;        /* True if the timer is a total timer, false otherwise. */

    ScopedTimer(const std::string& id, const bool total = false) : id(id), total(total) {
      if (total) console::total_start(id);
      else console::average_start(id);
    }

    ~ScopedTimer() {
      if (total) console::total_end(id);
      else console::average_end(id);
    }
  };

}

namespace std {

  template <typename T>
  inline ostream& operator<<(ostream& os, const Graphick::Math::Vec2<T> v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
  }

  template <typename T>
  inline ostream& operator<<(ostream& os, const Graphick::Math::Vec3<T>& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << ")";
    return os;
  }

  template <typename T>
  inline ostream& operator<<(ostream& os, const Graphick::Math::Vec4<T>& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return os;
  }

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
