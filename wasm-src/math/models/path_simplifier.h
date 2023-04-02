#pragma once

#include "path_point.h"

#include <vector>

std::vector<uint> simplify_path(
  const std::vector<PathPoint>& path,
  uint start_index,
  uint end_index,
  float threshold
);

// typedef void (*HeapFreeFP)(void* ptr);

// struct HeapNode {
//   void* ptr;
//   float value;
//   uint index;
// };

// struct PoolChunk {
//   PoolChunk* prev;
//   uint size;
//   uint bufsize;
//   HeapNode buf[0];
// };

// struct PoolChunkElemFree {
//   PoolChunkElemFree* next;
// };

// struct HeapMemPool {
//   /* Always keep at least one chunk (never NULL) */
//   PoolChunk* chunk;
//   /* when NULL, allocate a new chunk */
//   PoolChunkElemFree* free;
// };

// struct Heap {
//   uint size;
//   uint bufsize;
//   HeapNode** tree;

//   HeapMemPool pool;
// };

// struct Knot {
//   Knot* next;
//   Knot* prev;

//   HeapNode* heap_node;

//   uint index;

//   bool can_remove;
//   bool is_removed;

//   vec2 handles;
//   float pressure;

//   float error_sq_next;

//   vec2* tan[2];
// };

// struct KnotRemoveParams {
//   const std::vector<PathPoint>& points;
//   Heap* heap;
// };

// struct KnotRemoveState {
//   uint index;
//   /* Handles for prev/next knots */
//   vec2 handles;
// };

struct Knot {
  Knot* prev;
  Knot* next;

  vec2 left;
  vec2 position;
  vec2 right;

  bool can_remove;
  bool is_removed;

  float pressure;
};

uint simplify_spline(
  Knot* knots, const uint knots_len, uint knots_len_remaining,
  const float error_sq_max
);
