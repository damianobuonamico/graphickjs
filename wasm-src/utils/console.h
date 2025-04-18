/**
 * @file console.h
 * @brief The file contains the definition of the console utility.
 *
 * @todo refactor timers and debugger
 */

#pragma once

#include "assert.h"

#include "../math/vec2.h"
#include "../math/vec3.h"
#include "../math/vec4.h"

#include <bitset>
#include <numeric>
#include <ostream>
#include <sstream>
#include <stdio.h>

namespace graphick::utils {

/**
 * @brief The class that represents the console utility.
 *
 * The console utility is responsible for logging messages and measuring time.
 */
struct console {
 public:
  /**
   * @brief Logs a message to the console.
   *
   * @param value The message to log.
   */
  template<typename T>
  static inline void log(T value)
  {
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
  template<typename T>
  static inline void log(const std::string& name, T value)
  {
    std::stringstream stream;
    stream << name << ": " << value;

    printf("%s\n", stream.str().c_str());
  }

  /**
   * @brief Logs an info message to the console.
   *
   * @param value The info message to log.
   */
  static inline void info(const std::string& value)
  {
    std::stringstream stream;
    stream << value;

    printf("\033[0;36m");
    printf("%s\n", stream.str().c_str());
    printf("\033[0m");
  }

  /**
   * @brief Logs an info message to the console.
   *
   * @param name The name of the info message.
   * @param value The info message to log.
   */
  static inline void info(const std::string& name, const std::string& value)
  {
    std::stringstream stream;
    stream << name << ": " << value;

    printf("\033[0;36m");
    printf("%s\n", stream.str().c_str());
    printf("\033[0m");
  }

  /**
   * @brief Logs a warning message to the console.
   *
   * @param value The warning message to log.
   */
  static inline void warn(const std::string& value)
  {
    std::stringstream stream;
    stream << value;

    printf("\033[0;33m");
    printf("%s\n", stream.str().c_str());
    printf("\033[0m");
  }

  /**
   * @brief Logs a warning message to the console.
   *
   * @param name The name of the warning message.
   * @param value The warning message to log.
   */
  static inline void warn(const std::string& name, const std::string& value)
  {
    std::stringstream stream;
    stream << name << ": " << value;

    printf("\033[0;33m");
    printf("%s\n", stream.str().c_str());
    printf("\033[0m");
  }

  /**
   * @brief Logs an error message to the console.
   *
   * @param value The error message to log.
   */
  template<typename T>
  static inline void error(T value)
  {
    std::stringstream stream;
    stream << value;

    printf("\033[0;31m");
    printf("%s\n", stream.str().c_str());
    printf("\033[0m");
  }

  /**
   * @brief Logs an error message to the console.
   *
   * @param name The name of the error message.
   * @param value The error message to log.
   */
  template<typename T>
  static inline void error(const std::string& name, T value)
  {
    std::stringstream stream;
    stream << name << ": " << value;

    printf("\033[0;31m");
    printf("%s\n", stream.str().c_str());
    printf("\033[0m");
  }

  /**
   * @brief Logs a bitset to the console.
   *
   * @param value The value to bitset.
   */
  template<typename T>
  static inline void bitset(T value)
  {
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
  template<typename T>
  static inline void bitset(const std::string& name, T value)
  {
    std::stringstream stream;
    stream << name << ": " << std::bitset<8 * sizeof(value)>(value);

    printf("%s\n", stream.str().c_str());
  }
};

}  // namespace graphick::utils

namespace std {

template<typename T>
inline ostream& operator<<(ostream& os, const graphick::math::Vec2<T> v)
{
  os << "(" << v.x << ", " << v.y << ")";
  return os;
}

template<typename T>
inline ostream& operator<<(ostream& os, const graphick::math::Vec3<T>& v)
{
  os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << ")";
  return os;
}

template<typename T>
inline ostream& operator<<(ostream& os, const graphick::math::Vec4<T>& v)
{
  os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
  return os;
}

}  // namespace std

namespace graphick {
using console = utils::console;
}
