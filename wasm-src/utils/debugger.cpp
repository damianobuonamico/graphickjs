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
#  define now() size_t(to_nanoseconds(std::chrono::steady_clock::now().time_since_epoch()).count())

namespace graphick::utils {

/* -- Static Members -- */

inline static std::unordered_map<std::string, debugger::DebugValue> s_values = {};
inline static std::unordered_map<std::string, debugger::TotalTimer> s_total_timers = {};
inline static std::unordered_map<std::string, debugger::AverageTimer> s_average_timers = {};

inline static vec4 s_severity_colors[] = {
    vec4(0.2f, 0.8f, 0.8f, 1.0f),     // Blue
    vec4(0.2f, 0.8f, 0.2f, 1.0f),     // Green
    vec4(0.95f, 0.67f, 0.11f, 1.0f),  // Orange
    vec4(0.8f, 0.2f, 0.2f, 1.0f),     // Red
};

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

size_t debugger::TotalTimer::average() const
{
  return std::reduce(records.begin(), records.end()) / RECORDS_COUNT;
}

/* -- debugger -- */

void debugger::frame()
{
  for (auto& [name, timer] : s_total_timers) {
    timer.next();
  }
}

void debugger::value(const std::string& name, const std::string& value)
{
  s_values[name] = DebugValue{value, now()};
}

void debugger::value_counter(const std::string& name, const int value)
{
  const std::string counter_name = "[counter] " + name;

  auto it = s_values.find(counter_name);

  if (it == s_values.end()) {
    debugger::value(counter_name, std::to_string(value));
  } else {
    it->second.value = std::to_string(std::stoi(it->second.value) + value);
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
  if (it == s_total_timers.end()) {
    return;
  }

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
  const size_t time = now();

  const vec2 screen_size = vec2(renderer::Renderer::viewport_size());

  const float font_size = 11.0f;
  const float line_height = std::ceil(font_size * 1.5f);
  const float padding = font_size;

  vec2 cursor = vec2(padding, line_height);
  vec2 size = vec2(0, 1.0f);

  std::vector<std::string> to_remove;

  for (const auto& [name, value] : s_values) {
    if (time - value.last_time > 1000000 * 17 * 60) {
      to_remove.push_back(name);
    }
  }

  for (const auto& name : to_remove) {
    s_values.erase(name);
  }

  for (const auto& [name, timer] : s_total_timers) {
    const size_t average = timer.average();

    if (average == 0) {
      continue;
    }

    const double ms = static_cast<double>(average) / 1e6;
    const std::string text = name + ": " + std::to_string(ms) + " ms";

    size.x = std::max(size.x, __debug_text(text, cursor, vec4::zero()));
    cursor.y += line_height;
  }

  to_remove.clear();

  for (const auto& [name, value] : s_values) {
    const std::string text = name + ": " + value.value;

    size.x = std::max(size.x, __debug_text(text, cursor, vec4::zero()));
    cursor.y += line_height;

    if (name.substr(0, 9) == "[counter]") {
      to_remove.push_back(name);
    }
  }

  size += cursor + padding;
  size.y -= line_height + 1.0f;

  cursor = vec2(screen_size.x - size.x, 0.0f);

  __debug_rect(rect(cursor, cursor + size), vec4(0.0f, 0.0f, 0.0f, 0.5f));

  cursor += vec2(padding, line_height);

  for (const auto& [name, timer] : s_total_timers) {
    const size_t average = timer.average();

    if (average == 0) {
      continue;
    }

    const double ms = static_cast<double>(average) / 1e6;
    const std::string text = name + ": ";
    const std::string value = std::to_string(ms) + " ms";

    vec4 color = vec4::identity();

    if (name.find("render") != std::string::npos || name.find("main") != std::string::npos ||
        name.find("GPU") != std::string::npos)
    {
      if (ms > 20.0) {
        color = s_severity_colors[3];
      } else if (ms > 17.5) {
        color = s_severity_colors[2];
      } else {
        color = s_severity_colors[1];
      }
    } else {
      if (ms > 3.0) {
        color = s_severity_colors[3];
      } else if (ms > 2.0) {
        color = s_severity_colors[2];
      } else {
        color = s_severity_colors[1];
      }
    }

    const float offset = __debug_text(text, cursor, vec4::identity());

    __debug_text(value, cursor + vec2(offset, 0.0f), color);

    cursor.y += line_height;
  }

  for (const auto& [name, value] : s_values) {
    const std::string text = name + ": ";
    const std::string value_text = value.value;

    const float offset = __debug_text(text, cursor, vec4::identity());

    __debug_text(value_text, cursor + vec2(offset, 0.0f), s_severity_colors[0]);

    cursor.y += line_height;
  }

  for (const auto& name : to_remove) {
    s_values.erase(name);
  }
}

}  // namespace graphick::utils

#endif
