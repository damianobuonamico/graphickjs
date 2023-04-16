#pragma once

#include <stdio.h>
#include <sstream>
#include <ostream>
#include <chrono>

struct console {
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
    last_time = std::chrono::high_resolution_clock::now();
  }

  static inline void time_end(const std::string& name = "Time Elapsed") {
    log(name, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - last_time).count());
  }
private:
  static inline std::chrono::steady_clock::time_point last_time;
};