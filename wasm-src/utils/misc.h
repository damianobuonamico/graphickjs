#pragma once

#include <initializer_list>

namespace Graphick::Utils {

  template<typename T, typename Fn>
  const Fn for_each(const T& first, const T& second, Fn func) {
    for (auto it = first.begin(); it != first.end(); it++) {
      func(*it);
    }
    for (auto it = second.begin(); it != second.end(); it++) {
      func(*it);
    }

    return func;
  }

}
