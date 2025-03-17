/**
 * @file hierarchy.h
 * @brief Contains the definition of the Hierarchy class.
 */

#pragma once

#include "../../math/mat2x3.h"
#include "../../utils/uuid.h"

#include <vector>

namespace graphick::editor {

struct HierarchyEntry {
  uuid id = uuid::null;

  bool is_layer = false;
  bool selected = false;

  mat2x3 transform = mat2x3::identity();
};

struct Hierarchy {
  std::vector<HierarchyEntry> entries;

  inline bool selected() const
  {
    return !entries.empty() && entries.back().selected;
  }

  inline mat2x3 transform() const
  {
    return entries.empty() ? mat2x3::identity() : entries.back().transform;
  }

  inline void push(const HierarchyEntry& entry)
  {
    entries.push_back(
        {entry.id,
         entry.is_layer,
         entry.selected || selected(),
         entries.empty() ? entry.transform : entries.back().transform * entry.transform});
  }

  inline void pop()
  {
    entries.pop_back();
  }

  inline void clear()
  {
    entries.clear();
  }
};

}  // namespace graphick::editor
