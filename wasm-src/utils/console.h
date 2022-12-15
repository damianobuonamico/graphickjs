#pragma once

#include <stdio.h>
#include <sstream>
#include <ostream>

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
};