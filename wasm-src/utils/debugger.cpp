/**
 * @file utils/debugger.cpp
 * @brief The file contains the implementation of the debugger utilities.
 */

#include "debugger.h"

#ifdef GK_DEBUG

#  include <chrono>
#  include <numeric>
#  include <unordered_map>

#  include "../renderer/renderer.h"

#  define to_nanoseconds(time_point) \
    std::chrono::duration_cast<std::chrono::nanoseconds>(time_point)
#  define now() to_nanoseconds(std::chrono::steady_clock::now().time_since_epoch()).count()

namespace graphick::utils {

/* -- Static Members -- */

inline static std::unordered_map<std::string, debugger::TotalTimer> s_total_timers = {};
inline static std::unordered_map<std::string, debugger::AverageTimer> s_average_timers = {};

/* -- TotalTimer -- */

void debugger::TotalTimer::start()
{
  last_time = now();
}

void debugger::TotalTimer::end()
{
  end(now() - last_time);
}

void debugger::TotalTimer::end(const size_t record)
{
  records[index % RECORDS_COUNT] += record;
}

void debugger::TotalTimer::next()
{
  index++;
  records[index % RECORDS_COUNT] = 0;
}

size_t debugger::TotalTimer::average()
{
  return std::reduce(records.begin(), records.end()) / RECORDS_COUNT;
}

/* -- debugger -- */

void debugger::frame()
{
  // total_end("FrameTime");
  for (auto& [name, timer] : s_total_timers) {
    timer.next();
  }
}

void debugger::total_start(const std::string& name)
{
  auto it = s_total_timers.find(name);
  size_t time = now();

  if (it == s_total_timers.end()) {
    TotalTimer timer;
    timer.start();

    s_total_timers.insert({name, timer});
  } else {
    it->second.start();
  }
}

void debugger::total_end(const std::string& name)
{
  auto it = s_total_timers.find(name);
  if (it == s_total_timers.end())
    return;

  it->second.end();
}

void debugger::total_record(const std::string& name, const size_t record)
{
  auto it = s_total_timers.find(name);
  size_t time = now();

  if (it == s_total_timers.end()) {
    TotalTimer timer;
    timer.end(record);

    s_total_timers.insert({name, timer});
  } else {
    it->second.end(record);
  }
}

void debugger::average_start(const std::string& name)
{
  auto it = s_average_timers.find(name);
  size_t time = now();

  if (it == s_average_timers.end()) {
    s_average_timers.insert({name, {time}});
  } else {
    it->second.last_time = time;
  }
}

void debugger::average_end(const std::string& name)
{
  auto it = s_average_timers.find(name);
  size_t time = now();

  if (it == s_average_timers.end()) {
    return;
  }

  size_t duration = time - it->second.last_time;

  it->second.duration = (it->second.duration * it->second.samples + duration) /
                        (it->second.samples + 1);
  it->second.samples++;
}

void debugger::render()
{
  __debug_rect(rect(0, 0, 100, 100), vec4(0.3f, 0.3f, 0.3f, 0.3f));
}

}  // namespace graphick::utils

#endif
