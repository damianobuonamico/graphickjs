/*
 * Original implementation:
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 */

#pragma once

#include "../../math/vector.h"
#include "../../math/matrix.h"
#include "../../utils/console.h"

#include <vector>

#define MAXPOINTS	1000

 // TODO: Move
struct FreehandPathPoint {
  vec2 position;
  float pressure;
};

using PathPoints = std::vector<FreehandPathPoint>;

struct Bezier {
  vec2 p0;
  vec2 p1;
  vec2 p2;
  vec2 p3;

  float p0_pressure = 1.0f;
  float p3_pressure = 1.0f;

  vec2& operator[](uint8_t i) {
    switch (i) {
    default:
    case 0:
      return p0;
    case 1:
      return p1;
    case 2:
      return p2;
    case 3:
      return p3;
    }
  }

  const vec2& operator[](uint8_t i) const {
    switch (i) {
    default:
    case 0:
      return p0;
    case 1:
      return p1;
    case 2:
      return p2;
    case 3:
      return p3;
    }
  }
};

static const vec2* operator&(const Bezier& b) { return (vec2*)&(b.p0); }

/*
 * B0, B1, B2, B3 :
 * Bezier multipliers
 */
static float B0(float u) {
  float tmp = 1.0f - u;
  return (tmp * tmp * tmp);
}

static float B1(float u) {
  float tmp = 1.0f - u;
  return (3.0f * u * (tmp * tmp));
}

static float B2(float u) {
  float tmp = 1.0f - u;
  return (3.0f * u * u * tmp);
}

static float B3(float u) {
  return (u * u * u);
}

/*
 * generate_bezier:
 * Use least-squares method to find Bezier control points for region.
 */
static Bezier generate_bezier(
  const PathPoints& points, const size_t first, const size_t last,
  const std::vector<float>& u_prime,
  const vec2& t_hat_1, const vec2& t_hat_2
) {
  size_t i;
  vec2 A[MAXPOINTS][2];	/* Precomputed rhs for eqn */
  size_t n_pts; /* Number of pts in sub-curve */
  mat2 C{ 0.0f }; /* Matrix C */
  vec2 X{ 0.0f }; /* Matrix X */
  float det_C0_C1, det_C0_X, det_X_C1; /* Determinants of matrices */
  float alpha_l, alpha_r; /* Alpha values, left and right	*/
  vec2 tmp;			/* Utility variable	*/
  Bezier bez_curve;	/* RETURN bezier curve ctl pts */
  float seg_length;
  float epsilon;

  n_pts = last - first + 1;

  /* Compute the A's*/
  for (i = 0; i < n_pts; i++) {
    A[i][0] = t_hat_1 * B1(u_prime[i]);
    A[i][1] = t_hat_2 * B2(u_prime[i]);
  }

  for (i = 0; i < n_pts; i++) {
    C[0][0] += dot(A[i][0], A[i][0]);
    C[0][1] += dot(A[i][0], A[i][1]);
    C[1][0] = C[0][1];
    C[1][1] += dot(A[i][1], A[i][1]);

    tmp = points[first + i].position - (
      points[first].position * B0(u_prime[i]) +
      points[first].position * B1(u_prime[i]) +
      points[last].position * B2(u_prime[i]) + points[last].position * B3(u_prime[i]));

    X[0] += dot(A[i][0], tmp);
    X[1] += dot(A[i][1], tmp);
  }

  /* Compute the determinants of C and X (Cramer)	*/
  det_C0_C1 = determinant(C);
  det_C0_X = C[0][0] * X[1] - C[1][0] * X[0];
  det_X_C1 = X[0] * C[1][1] - X[1] * C[0][1];

  /* Finally, derive alpha values	*/
  alpha_l = (det_C0_C1 == 0.0f) ? 0.0f : det_X_C1 / det_C0_C1;
  alpha_r = (det_C0_C1 == 0.0f) ? 0.0f : det_C0_X / det_C0_C1;

  /* If alpha negative, use the Wu/Barsky heuristic (see text)
   * (if alpha is 0, you get coincident control points that lead to
   * divide by zero in any subsequent NewtonRaphsonRootFind() call. */
  seg_length = distance(points[last].position, points[first].position);
  epsilon = 1.0e-6f * seg_length;
  if (alpha_l < epsilon || alpha_r < epsilon) {
    /* fall back on standard (probably inaccurate) formula, and subdivide further if needed. */
    float dist = seg_length / 3.0f;

    bez_curve.p0 = points[first].position;
    bez_curve.p3 = points[last].position;
    bez_curve.p1 = bez_curve.p0 + t_hat_1 * dist;
    bez_curve.p2 = bez_curve.p3 + t_hat_2 * dist;

    return bez_curve;
  }

  /* First and last control points of the Bezier curve are */
  /* positioned exactly at the first and last data points */
  /* Control points 1 and 2 are positioned an alpha distance out */
  /* on the tangent vectors, left and right, respectively */
  bez_curve.p0 = points[first].position;
  bez_curve.p3 = points[last].position;
  bez_curve.p1 = bez_curve.p0 + t_hat_1 * alpha_l;
  bez_curve.p2 = bez_curve.p3 + t_hat_2 * alpha_r;

  bez_curve.p0_pressure = points[first].pressure;
  bez_curve.p3_pressure = points[last].pressure;

  return (bez_curve);
}

/*
 * Bezier:
 * Evaluate a Bezier curve at a particular parameter value
 */
static vec2 BII(int degree, const vec2* V, float t) {
  int i, j;
  vec2 Q;
  vec2* V_temp;

  V_temp = new vec2[degree + 1];

  for (i = 0; i <= degree; i++) {
    V_temp[i] = V[i];
  }

  /* Triangle computation	*/
  for (i = 1; i <= degree; i++) {
    for (j = 0; j <= degree - i; j++) {
      V_temp[j].x = (1.0f - t) * V_temp[j].x + t * V_temp[j + 1].x;
      V_temp[j].y = (1.0f - t) * V_temp[j].y + t * V_temp[j + 1].y;
    }
  }

  Q = V_temp[0];
  delete[] V_temp;

  return Q;
}

/*
 * newton_raphson_root_find :
 * Use Newton-Raphson iteration to find better root.
 */
static float newton_raphson_root_find(const Bezier& Q, const vec2& P, const float u) {
  float numerator, denominator;
  vec2 Q1[3], Q2[2]; /* Q' and Q'' */
  vec2 Q_u, Q1_u, Q2_u; /* u evaluated at Q, Q', & Q'' */
  float u_prime; /* Improved u */

  /* Compute Q(u)	*/
  Q_u = BII(3, &Q, u);

  /* Generate control vertices for Q'	*/
  for (int i = 0; i <= 2; i++) {
    Q1[i].x = (Q[i + 1].x - Q[i].x) * 3.0f;
    Q1[i].y = (Q[i + 1].y - Q[i].y) * 3.0f;
  }

  /* Generate control vertices for Q'' */
  for (int i = 0; i <= 1; i++) {
    Q2[i].x = (Q1[i + 1].x - Q1[i].x) * 2.0f;
    Q2[i].y = (Q1[i + 1].y - Q1[i].y) * 2.0f;
  }

  /* Compute Q'(u) and Q''(u)	*/
  Q1_u = BII(2, Q1, u);
  Q2_u = BII(1, Q2, u);

  /* Compute f(u)/f'(u) */
  numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
  denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
    (Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);
  if (denominator == 0.0f) return u;

  /* u = u - f(u)/f'(u) */
  u_prime = u - (numerator / denominator);
  return u_prime;
}

/*
 * reparameterize:
 * Given set of points and their parameterization, try to find
 * a better parameterization.
 */
static std::vector<float> reparameterize(
  const PathPoints& points, const size_t first, const size_t last,
  const std::vector<float>& u, const Bezier& bez_curve
) {
  size_t n_pts = last - first + 1;
  std::vector<float> u_prime(n_pts); /* New parameter values */

  for (size_t i = first; i <= last; i++) {
    u_prime[i - first] = newton_raphson_root_find(bez_curve, points[i].position, u[i - first]);
  }
  return u_prime;
}

/*
 * compute_left_tangent, compute_right_tangent, compute_center_tangent:
 * Approximate unit tangents at endpoints and "center" of digitized curve
 */
static vec2 compute_left_tangent(const PathPoints& points, size_t end) {
  vec2 t_hat_1;

  t_hat_1 = points[end + 1].position - points[end].position;
  t_hat_1 = normalize(t_hat_1);

  return t_hat_1;
}

static vec2 compute_right_tangent(const PathPoints& points, size_t end) {
  vec2 t_hat_2;

  t_hat_2 = points[end - 1].position - points[end].position;
  t_hat_2 = normalize(t_hat_2);

  return t_hat_2;
}


static vec2 compute_center_tangent(const PathPoints& points, size_t center) {
  vec2	v1, v2, t_hat_center;

  v1 = points[center - 1].position - points[center].position;
  v2 = points[center].position - points[center + 1].position;

  t_hat_center = (v1 + v2) / 2.0f;
  t_hat_center = normalize(t_hat_center);

  return t_hat_center;
}


/*
 * chord_length_parameterize:
 * Assign parameter values to digitized points
 * using relative distances between points.
 */
static std::vector<float> chord_length_parameterize(const PathPoints& points, size_t first, size_t last) {
  std::vector<float> u(last - first + 1); /* Parameterization */
  size_t i;

  u[0] = 0.0f;
  for (i = first + 1; i <= last; i++) {
    u[i - first] = u[i - first - 1] + distance(points[i].position, points[i - 1].position);
  }

  for (i = first + 1; i <= last; i++) {
    u[i - first] = u[i - first] / u[last - first];
  }

  return u;
}

/*
 * compute_max_error:
 * Find the maximum squared distance of digitized points
 * to fitted curve.
 */
static float compute_max_error(
  const PathPoints& points, const size_t first, const size_t last,
  const Bezier& bez_curve, const std::vector<float>& u, size_t* split_point
) {
  size_t i;
  float	max_dist; /* Maximum error */
  float	dist; /* Current error */
  vec2 P; /* Point on curve	*/
  vec2 v; /* Vector from point to curve	*/

  *split_point = (last - first + 1) / 2;
  max_dist = 0.0f;

  for (i = first + 1; i < last; i++) {
    P = BII(3, &bez_curve, u[i - first]);
    v = P - points[i].position;
    dist = squared_length(v);

    if (dist >= max_dist) {
      max_dist = dist;
      *split_point = i;
    }
  }

  return max_dist;
}

static void fit_cubic(
  const PathPoints& points, size_t first, size_t last,
  vec2 t_hat_1, vec2 t_hat_2, float error,
  std::vector<Bezier>& r_curves
) {
  Bezier bez_curve; /* Control points of fitted Bezier curve */
  std::vector<float> u; /* Parameter values for point */
  std::vector<float> u_prime; /* Improved parameter values */
  float	max_error; /* Maximum fitting error */
  size_t split_point; /* Point to split point set at */
  size_t n_pts = last - first + 1; /* Number of points in subset */
  float	iteration_error = error * 4.0f; /* Error below which you try iterating */
  int max_iterations = 4; /* Max times to try iterating */
  vec2 t_hat_center; /* Unit tangent vector at split_point */

  /* Use heuristic if region only has two points in it */
  if (n_pts == 2) {
    float dist = distance(points[last].position, points[first].position) / 3.0f;

    bez_curve.p0 = points[first].position;
    bez_curve.p3 = points[last].position;
    bez_curve.p1 = bez_curve.p0 + t_hat_1 * dist;
    bez_curve.p2 = bez_curve.p3 + t_hat_2 * dist;

    r_curves.push_back(bez_curve);

    return;
  }

  /* Parameterize points, and attempt to fit curve */
  u = chord_length_parameterize(points, first, last);
  bez_curve = generate_bezier(points, first, last, u, t_hat_1, t_hat_2);

  /* Find max deviation of points to fitted curve */
  max_error = compute_max_error(points, first, last, bez_curve, u, &split_point);
  if (max_error < error) {
    r_curves.push_back(bez_curve);
    return;
  }

  /* If error not too large, try some reparameterization
   * and iteration
   */
  if (max_error < iteration_error) {
    for (size_t i = 0; i < max_iterations; i++) {
      u_prime = reparameterize(points, first, last, u, bez_curve);
      bez_curve = generate_bezier(points, first, last, u_prime, t_hat_1, t_hat_2);
      max_error = compute_max_error(points, first, last, bez_curve, u_prime, &split_point);

      if (max_error < error) {
        r_curves.push_back(bez_curve);
        return;
      }

      u.swap(u_prime);
    }
  }

  /* Fitting failed -- split at max error point and fit recursively */
  t_hat_center = compute_center_tangent(points, split_point);
  fit_cubic(points, first, split_point, t_hat_1, t_hat_center, error, r_curves);
  negate(t_hat_center, t_hat_center);
  fit_cubic(points, split_point, last, t_hat_center, t_hat_2, error, r_curves);
}

static std::vector<Bezier> fit_to_bezier_curves(const PathPoints& points, const size_t start, const size_t end, const float error) {
  // std::vector<std::vector<float>> parameterization = initialize_parameterization(points, start, end);

  vec2 t_hat_1 = compute_left_tangent(points, start);
  vec2 t_hat_2 = compute_right_tangent(points, end);

  std::vector<Bezier> curves;

  fit_cubic(points, start, end, t_hat_1, t_hat_2, error, curves);

  return curves;
}

// #define POOL_CHUNK_DEFAULT_NUM (((1 << 16) - sizeof(PoolChunk)) / sizeof(HeapNode))

// static PoolChunk* pool_alloc_chunk(uint tot_elems, PoolChunk* chunk_prev) {
//   PoolChunk* chunk = (PoolChunk*)malloc(sizeof(PoolChunk) + (sizeof(HeapNode) * tot_elems));

//   chunk->prev = chunk_prev;
//   chunk->bufsize = tot_elems;
//   chunk->size = 0;

//   return chunk;
// }

// static HeapNode* heap_pool_elem_alloc(HeapMemPool* pool) {
//   HeapNode* elem;

//   if (pool->free) {
//     elem = (HeapNode*)pool->free;
//     pool->free = pool->free->next;
//   } else {
//     PoolChunk* chunk = pool->chunk;
//     if (chunk->size == chunk->bufsize) {
//       chunk = pool->chunk = pool_alloc_chunk(POOL_CHUNK_DEFAULT_NUM, chunk);
//     }
//     elem = &chunk->buf[chunk->size++];
//   }

//   return elem;
// }

// static void heap_pool_create(HeapMemPool* pool, uint tot_reserve) {
//   pool->chunk = pool_alloc_chunk((tot_reserve > 1) ? tot_reserve : POOL_CHUNK_DEFAULT_NUM, nullptr);
//   pool->free = nullptr;
// }

// static void heap_pool_elem_free(HeapMemPool* pool, HeapNode* elem) {
//   PoolChunkElemFree* elem_free = (PoolChunkElemFree*)elem;
//   elem_free->next = pool->free;
//   pool->free = elem_free;
// }

// static void heap_pool_destroy(HeapMemPool* pool) {
//   PoolChunk* chunk = pool->chunk;
//   do {
//     PoolChunk* chunk_prev;
//     chunk_prev = chunk->prev;
//     free(chunk);
//     chunk = chunk_prev;
//   } while (chunk);

//   pool->chunk = nullptr;
//   pool->free = nullptr;
// }

// /* swap with a temp value */
// #define SWAP_TVAL(tval, a, b)  {  \
// 	(tval) = (a);                 \
// 	(a) = (b);                    \
// 	(b) = (tval);                 \
// } (void)0

// #define HEAP_PARENT(i) (((i) - 1) >> 1)
// #define HEAP_LEFT(i)   (((i) << 1) + 1)
// #define HEAP_RIGHT(i)  (((i) << 1) + 2)
// #define HEAP_COMPARE(a, b) ((a)->value < (b)->value)

// static Heap* heap_new(uint tot_reserve) {
//   Heap* heap = new Heap;
//   /* ensure we have at least one so we can keep doubling it */
//   heap->size = 0;
//   heap->bufsize = tot_reserve ? tot_reserve : 1;
//   heap->tree = new HeapNode * [heap->bufsize];

//   heap_pool_create(&heap->pool, tot_reserve);
// }

// static void heap_free(Heap* heap, HeapFreeFP ptrfreefp) {
//   if (ptrfreefp) {
//     uint i;

//     for (i = 0; i < heap->size; i++) {
//       ptrfreefp(heap->tree[i]->ptr);
//     }
//   }

//   heap_pool_destroy(&heap->pool);

//   free(heap->tree);
//   free(heap);
// }

// static bool heap_is_empty(const Heap* heap) {
//   return (heap->size == 0);
// }

// static float heap_top_value(const Heap* heap) {
//   return heap->tree[0]->value;
// }

// static void heap_swap(Heap* heap, const uint i, const uint j) {
//   HeapNode** tree = heap->tree;
//   union {
//     uint      index;
//     HeapNode* node;
//   } tmp;
//   SWAP_TVAL(tmp.index, tree[i]->index, tree[j]->index);
//   SWAP_TVAL(tmp.node, tree[i], tree[j]);
// }

// static void heap_down(Heap* heap, uint i) {
//   /* size won't change in the loop */
//   const uint size = heap->size;

//   while (1) {
//     const uint l = HEAP_LEFT(i);
//     const uint r = HEAP_RIGHT(i);
//     uint smallest;

//     smallest = ((l < size) && HEAP_COMPARE(heap->tree[l], heap->tree[i])) ? l : i;

//     if ((r < size) && HEAP_COMPARE(heap->tree[r], heap->tree[smallest])) {
//       smallest = r;
//     }

//     if (smallest == i) {
//       break;
//     }

//     heap_swap(heap, i, smallest);
//     i = smallest;
//   }
// }

// static void heap_up(Heap* heap, uint i) {
//   while (i > 0) {
//     const uint p = HEAP_PARENT(i);

//     if (HEAP_COMPARE(heap->tree[p], heap->tree[i])) {
//       break;
//     }
//     heap_swap(heap, p, i);
//     i = p;
//   }
// }


// static HeapNode* heap_insert(Heap* heap, float value, void* ptr) {
//   HeapNode* node;

//   if (heap->size >= heap->bufsize) {
//     heap->bufsize *= 2;
//     heap->tree = (HeapNode**)realloc(heap->tree, heap->bufsize * sizeof(*heap->tree));
//   }

//   node = heap_pool_elem_alloc(&heap->pool);

//   node->ptr = ptr;
//   node->value = value;
//   node->index = heap->size;

//   heap->tree[node->index] = node;

//   heap->size++;

//   heap_up(heap, node->index);

//   return node;
// }

// static void* heap_popmin(Heap* heap) {
//   void* ptr = heap->tree[0]->ptr;

//   assert(heap->size != 0);

//   heap_pool_elem_free(&heap->pool, heap->tree[0]);

//   if (--heap->size) {
//     heap_swap(heap, 0, heap->size);
//     heap_down(heap, 0);
//   }

//   return ptr;
// }

// static void heap_remove(Heap* heap, HeapNode* node) {
//   uint i = node->index;

//   assert(heap->size != 0);

//   while (i > 0) {
//     uint p = HEAP_PARENT(i);

//     heap_swap(heap, p, i);
//     i = p;
//   }

//   heap_popmin(heap);
// }

// static void heap_node_value_update(Heap* heap, HeapNode* node, float value) {
//   assert(heap->size != 0);
//   if (node->value == value) {
//     return;
//   }
//   node->value = value;
//   /* Can be called in either order, makes no difference. */
//   heap_up(heap, node->index);
//   heap_down(heap, node->index);
// }

// static void heap_node_value_update_ptr(Heap* heap, HeapNode* node, float value, void* ptr) {
//   node->ptr = ptr;
//   heap_node_value_update(heap, node, value);
// }

// static void* heap_node_ptr(HeapNode* node) {
//   return node->ptr;
// }

// void heap_insert_or_update(Heap* heap, HeapNode** node_p, float value, void* ptr) {
//   if (*node_p == NULL) {
//     *node_p = heap_insert(heap, value, ptr);
//   } else {
//     heap_node_value_update_ptr(heap, *node_p, value, ptr);
//   }
// }

// static float knot_remove_error_value(
//   const vec2* tan_l, const vec2* tan_r,
//   const std::vector<PathPoint>& points,
//   const uint points_offset, const uint points_offset_len,
//   /* Avoid having to re-calculate again */
//   vec2 r_handle_factors, uint* r_error_index
// ) {
//   float error_sq = FLT_MAX;

//   vec2 handle_factor_l, handle_factor_r;

//   fit_cubic(
//     points, points_offset, points_offset_len,
//     tan_l, tan_r, error_sq, 
//   )

//   fit_curve(
//     points_offset, points_offset_len, points_offset_length_cache, dims, 0.0,
//     tan_l, tan_r,
//     handle_factor_l, handle_factor_r,
//     &error_sq, r_error_index);

//   assert(error_sq != DBL_MAX);

//   handle_factor_l -= points[points_offset].position;
//   r_handle_factors.x = dot(*tan_l, handle_factor_l);

//   handle_factor_r -= points[points_offset_len - 1].position;
//   r_handle_factors.y = dot(*tan_r, handle_factor_r);

//   return error_sq;
// }

// static float knot_calc_curve_error_value(
//   const std::vector<PathPoint>& points,
//   const Knot* knot_l, const Knot* knot_r,
//   const vec2* tan_l, const vec2* tan_r,
//   vec2& r_handle_factors
// ) {
//   const uint points_offset_len = ((knot_l->index < knot_r->index) ?
//     (knot_r->index - knot_l->index) :
//     ((knot_r->index + points.size()) - knot_l->index)) + 1;

//   if (points_offset_len != 2) {
//     uint error_index_dummy;
//     return knot_remove_error_value(
//       tan_l, tan_r, points,
//       knot_l->index, points_offset_len,
//       r_handle_factors, &error_index_dummy
//     );
//   } else {
//     /* No points between, use 1/3 handle length with no error as a fallback. */
//     assert(points_offset_len == 2);
//     r_handle_factors = vec2{ distance(points[knot_l->index + 0].position, points[knot_l->index + 1].position) / 3.0f };

//     return 0.0f;
//   }
// }

// static void knot_remove_error_recalculate(KnotRemoveParams* p, Knot* k, const float error_sq_max) {
//   assert(k->can_remove);
//   vec2 handles;

//   const float cost_sq = knot_calc_curve_error_value(
//     p->points, k->prev, k->next,
//     k->prev->tan[1], k->next->tan[0],
//     handles
//   );

//   if (cost_sq < error_sq_max) {
//     KnotRemoveState* r;
//     if (k->heap_node) {
//       r = (KnotRemoveState*)heap_node_ptr(k->heap_node);
//     } else {
//       r = new KnotRemoveState;
//       r->index = k->index;
//     }

//     r->handles[0] = handles[0];
//     r->handles[1] = handles[1];

//     heap_insert_or_update(p->heap, &k->heap_node, cost_sq, r);
//   } else {
//     if (k->heap_node) {
//       KnotRemoveState* r;
//       r = (KnotRemoveState*)heap_node_ptr(k->heap_node);
//       heap_remove(p->heap, k->heap_node);

//       free(r);

//       k->heap_node = nullptr;
//     }
//   }
// }

// uint simplify_spline(
//   const std::vector<PathPoint>& points,
//   Knot* knots, const uint knots_len, uint knots_len_remaining,
//   const float error_sq_max
// ) {
//   Heap* heap = heap_new(knots_len_remaining);

//   KnotRemoveParams params = { points, heap };

//   for (uint i = 0; i < knots_len; i++) {
//     Knot* k = &knots[i];
//     if (k->can_remove && k->is_removed == false) {
//       knot_remove_error_recalculate(&params, k, error_sq_max);
//     }
//   }

//   while (heap_is_empty(heap) == false) {
//     Knot* k;

//     {
//       const float error_sq = heap_top_value(heap);
//       KnotRemoveState* r = (KnotRemoveState*)heap_popmin(heap);
//       k = &knots[r->index];
//       k->heap_node = nullptr;
//       k->prev->handles[1] = r->handles[0];
//       k->next->handles[0] = r->handles[1];

//       k->prev->error_sq_next = error_sq;

//       free(r);
//     }

//     if (knots_len_remaining <= 2) {
//       continue;
//     }

//     Knot* k_prev = k->prev;
//     Knot* k_next = k->next;

//     /* Remove ourselves */
//     k_next->prev = k_prev;
//     k_prev->next = k_next;

//     k->next = nullptr;
//     k->prev = nullptr;
//     k->is_removed = true;

//     if (k_prev->can_remove && (k_prev->prev && k_prev->next)) {
//       knot_remove_error_recalculate(&params, k_prev, error_sq_max);
//     }

//     if (k_next->can_remove && (k_next->prev && k_next->next)) {
//       knot_remove_error_recalculate(&params, k_next, error_sq_max);
//     }

//     knots_len_remaining -= 1;
//   }

//   heap_free(heap, free);

//   return knots_len_remaining;
// }