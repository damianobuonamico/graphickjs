/**
 * @file utils/debugger.h
 * @brief The file contains the definition of the debugger utilities.
 */

#pragma once

#include "defines.h"

#define RECORDS_COUNT 150

#ifdef GK_DEBUG

#  include <string>
#  include <vector>

namespace graphick::utils {

/**
 * @brief The class that represents the debugger utility.
 *
 * The debugger utility is responsible for creating timers and querying draw commands to the
 * renderer's debug layer.
 */
struct debugger {
 public:
  /**
   * @brief The structure that represents a timer used to measure the average time of a task.
   */
  struct AverageTimer {
    size_t last_time;     // The last time the timer was started.
    size_t duration = 0;  // The total duration of the task.
    size_t samples = 0;   // The number of samples.
  };

  /**
   * @brief The structure that represents a timer used to measure the total time of a task during a
   * frame.
   */
  struct TotalTimer {
    std::vector<size_t> records = std::vector<size_t>(RECORDS_COUNT);  // The records of the task.

    size_t last_time;  // The last time the timer was started.
    size_t index = 0;  // The index of the current record.

    /**
     * @brief Starts the timer.
     */
    void start();

    /**
     * @brief Ends the timer.
     */
    void end();

    /**
     * @brief Ends the timer with a specific time.
     *
     * @param record The time to record.
     */
    void end(const size_t record);

    /**
     * @brief Moves to the next record.
     */
    void next();

    /**
     * @brief Returns the average time of the task.
     *
     * @return The average time of the task.
     */
    size_t average() const;
  };

 public:
  /**
   * @brief Starts a new frame.
   */
  static void frame();

  /**
   * @brief Starts a total timer.
   *
   * @param name The name of the timer.
   */
  static void total_start(const std::string& name);

  /**
   * @brief Ends a total timer.
   *
   * @param name The name of the timer.
   */
  static void total_end(const std::string& name);

  /**
   * @brief Records an external total timer (e.g. GPU timer).
   *
   * @param name The name of the timer.
   * @param record The time to record.
   */
  static void total_record(const std::string& name, const size_t record);

  /**
   * @brief Starts an average timer.
   *
   * @param name The name of the timer.
   */
  static void average_start(const std::string& name);

  /**
   * @brief Ends an average timer.
   *
   * @param name The name of the timer.
   */
  static void average_end(const std::string& name);

  /**
   * @brief Issues the necessary draw commands to the renderer.
   */
  static void render();
};

/**
 * @brief The structure that represents a scoped timer.
 *
 * It is used by the __debug_time_total and __debug_time_average macros.
 */
struct ScopedTimer {
  std::string id;  // The name of the timer.
  bool total;      // True if the timer is a total timer, false otherwise.

  ScopedTimer(const std::string& id, const bool total = false) : id(id), total(total)
  {
    if (total) {
      debugger::total_start(id);
    } else {
      debugger::average_start(id);
    }
  }

  ~ScopedTimer()
  {
    if (total) {
      debugger::total_end(id);
    } else {
      debugger::average_end(id);
    }
  }
};

}  // namespace graphick::utils

namespace graphick {
using debugger = utils::debugger;
}

/**
 * @brief Creates a scoped timer and adds it to the total time of the task.
 */
#  define __debug_time_total() graphick::utils::ScopedTimer __scoped_timer(__FUNCTION__, true)

/**
 * @brief Adds the time to the task with the given name.
 */
#  define __debug_time_total_record(name, record) \
    graphick::utils::debugger::total_record(name, record)

/**
 * @brief Creates a scoped timer and averages it with the other entries of the task.
 */
#  define __debug_time_average() graphick::utils::ScopedTimer __scoped_timer(__FUNCTION__)

/**
 * @brief Sets up the debugger for the next frame.
 */
#  define __debug_time_frame() \
    graphick::utils::debugger::frame(); \
    __debug_time_total()

#else

#  define __debug_time_total() ((void)0)
#  define __debug_time_total_record(...) ((void)0)
#  define __debug_time_average() ((void)0)
#  define __debug_time_frame() ((void)0)

#endif
