/** Ported from source/blender/blenlib/intern/polyfill_2d.c
 *
 * An ear clipping algorithm to triangulate single boundary polygons.
 *
 * Details:
 *
 * - The algorithm guarantees all triangles are assigned (number of coords - 2)
 *   and that triangles will have non-overlapping indices (even for degenerate geometry).
 * - Self-intersections are considered degenerate (resulting triangles will overlap).
 * - While multiple polygons aren't supported, holes can still be defined using *key-holes*
 *   (where the polygon doubles back on itself with *exactly* matching coordinates).
 */

import { vec2 } from '@/math';

const DEBUG_TIME = true;

/* avoid fan-fill topology */
const USE_CLIP_EVEN = true;
const USE_CONVEX_SKIP = false;
/* sweep back-and-forth about convex ears (avoids lop-sided fans) */
const USE_CLIP_SWEEP = true;

// #ifdef USE_CONVEX_SKIP
const USE_KDTREE = false;
// #endif

// #ifdef USE_KDTREE
/**
 * Spatial optimization for point-in-triangle intersection checks.
 * The simple version of this algorithm is `O(n^2)` complexity
 * (every point needing to check the triangle defined by every other point),
 * Using a binary-tree reduces the complexity to `O(n log n)`
 * plus some overhead of creating the tree.
 *
 * This is a single purpose KDTree based on BLI_kdtree with some modifications
 * to better suit polyfill2d.
 * - #KDTreeNode2D is kept small (only 16 bytes),
 *   by not storing coords in the nodes and using index values rather than pointers
 *   to reference neg/pos values.
 *
 * - #kdtree2d_isect_tri is the only function currently used.
 *   This simply intersects a triangle with the kdtree points.
 *
 * - the KDTree is only built & used when the polygon is concave.
 */

// TODO: Maybe implement spatial hash tree

// #endif /* USE_KDTREE */

enum Sign {
  CONCAVE = -1,
  TANGENTIAL = 0,
  CONVEX = 1
}

interface PolyFill {
  indices: PolyIndex[] /* vertex aligned */;

  coords: vec2[];
  coords_num: number;
  // #ifdef USE_CONVEX_SKIP
  //   uint coords_num_concave;
  // #endif

  /* A polygon with n vertices has a triangulation of n-2 triangles. */
  tris: vec3[];
  tris_num: number;

  // #ifdef USE_KDTREE
  //   struct KDTree2D kdtree;
  // #endif
}

/** Circular double linked-list. */
interface PolyIndex {
  next: PolyIndex;
  prev: PolyIndex;
  index: number;
  sign: Sign;
}

function signum_enum(a: number): Sign {
  if (a > 0) {
    return 1;
  }
  if (a === 0) {
    return 0;
  }

  return -1;
}

/**
 * alternative version of #area_tri_signed_v2
 * needed because of float precision issues
 *
 * \note removes / 2 since its not needed since we only need the sign.
 */
function area_tri_signed_v2_alt_2x(v1: vec2, v2: vec2, v3: vec2): Sign {
  const d2 = vec2.sub(v2, v1);
  const d3 = vec2.sub(v3, v1);
  return d2[0] * d3[1] - d3[0] * d2[1];
}

function span_tri_v2_sign(v1: vec2, v2: vec2, v3: vec2): Sign {
  return signum_enum(area_tri_signed_v2_alt_2x(v3, v2, v1));
}

function cross_poly_v2(verts: vec2[], nr: number): number {
  let a: number;
  let cross: number;
  let co_curr: vec2, co_prev: vec2;

  /* The Trapezium Area Rule */
  co_prev = verts[nr - 1];
  cross = 0;

  for (a = 0; a < nr; a++) {
    co_curr = verts[a];
    cross += (co_curr[0] - co_prev[0]) * (co_curr[1] + co_prev[1]);
    co_prev = co_curr;
  }

  return cross;
}

function pf_coord_remove(pf: PolyFill, pi: PolyIndex): void {
  // #ifdef USE_KDTREE
  //   /* avoid double lookups, since convex coords are ignored when testing intersections */
  //   if (pf->kdtree.node_num) {
  //     kdtree2d_node_remove(&pf->kdtree, pi->index);
  //   }
  // #endif

  pi.next.prev = pi.prev;
  pi.prev.next = pi.next;

  if (pf.indices[0] === pi) {
    pf.indices.shift();
  }

  pf.coords_num--;
}

function pf_triangulate(pf: PolyFill): void {
  /* localize */
  let pi_ear: PolyIndex;

  let pi_ear_init: PolyIndex = pf.indices[0];

  let reverse: boolean = false;

  while (pf.coords_num > 3) {
    let pi_prev: PolyIndex, pi_next: PolyIndex;
    let sign_orig_prev: Sign, sign_orig_next: Sign;

    pi_ear = pf_ear_tip_find(pf, pi_ear_init, reverse);

    //     #ifdef USE_CONVEX_SKIP
    //     if (pi_ear->sign != CONVEX) {
    //       pf->coords_num_concave -= 1;
    //     }
    // #endif

    pi_prev = pi_ear.prev;
    pi_next = pi_ear.next;

    pf_ear_tip_cut(pf, pi_ear);

    /* The type of the two vertices adjacent to the clipped vertex may have changed. */
    sign_orig_prev = pi_prev.sign;
    sign_orig_next = pi_next.sign;

    /* check if any verts became convex the (else if)
     * case is highly unlikely but may happen with degenerate polygons */
    if (sign_orig_prev != Sign.CONVEX) {
      pf_coord_sign_calc(pf, pi_prev);
      // #ifdef USE_CONVEX_SKIP
      //   if (pi_prev->sign == CONVEX) {
      //     pf->coords_num_concave -= 1;
      // #  ifdef USE_KDTREE
      //     kdtree2d_node_remove(&pf->kdtree, pi_prev->index);
      // #  endif
      //   }
      // #endif
    }
    if (sign_orig_next != Sign.CONVEX) {
      pf_coord_sign_calc(pf, pi_next);
      // #ifdef USE_CONVEX_SKIP
      //   if (pi_next->sign == CONVEX) {
      //     pf->coords_num_concave -= 1;
      // #  ifdef USE_KDTREE
      //     kdtree2d_node_remove(&pf->kdtree, pi_next->index);
      // #  endif
      //   }
      // #endif
    }

    pi_ear_init = reverse ? pi_prev.prev : pi_next.next;

    if (pi_ear_init.sign != Sign.CONVEX) {
      /* take the extra step since this ear isn't a good candidate */
      pi_ear_init = reverse ? pi_ear_init.prev : pi_ear_init.next;
      reverse = !reverse;
    }
  }

  if (pf.coords_num == 3) {
    pi_ear = pf.indices[0];

    const tri: vec3 = [pi_ear.index, 0, 0];
    pi_ear = pi_ear.next;
    tri[1] = pi_ear.index;
    pi_ear = pi_ear.next;
    tri[2] = pi_ear.index;

    pf.tris.push(tri);
    pf.tris_num++;
  }
}

/**
 * \return CONCAVE, TANGENTIAL or CONVEX
 */
function pf_coord_sign_calc(pf: PolyFill, pi: PolyIndex): void {
  /* localize */
  const coords: vec2[] = pf.coords;

  pi.sign = span_tri_v2_sign(coords[pi.prev.index], coords[pi.index], coords[pi.next.index]);
}

function pf_ear_tip_find(pf: PolyFill, pi_ear_init: PolyIndex, reverse: boolean): PolyIndex {
  /* localize */
  const coords_num: number = pf.coords_num;
  let pi_ear: PolyIndex;

  let i: number;

  pi_ear = pi_ear_init;

  i = coords_num;
  while (i--) {
    if (pf_ear_tip_check(pf, pi_ear)) {
      return pi_ear;
    }
    pi_ear = reverse ? pi_ear.prev : pi_ear.next;
  }

  /* Desperate mode: if no vertex is an ear tip,
   * we are dealing with a degenerate polygon (e.g. nearly collinear).
   * Note that the input was not necessarily degenerate,
   * but we could have made it so by clipping some valid ears.
   *
   * Idea taken from Martin Held, "FIST: Fast industrial-strength triangulation of polygons",
   * Algorithmica (1998),
   * http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.115.291
   *
   * Return a convex or tangential vertex if one exists.
   */

  pi_ear = pi_ear_init;

  i = coords_num;
  while (i--) {
    if (pi_ear.sign != Sign.CONCAVE) {
      return pi_ear;
    }
    pi_ear = pi_ear.next;
  }

  /* If all vertices are concave, just return the last one. */
  return pi_ear;
}

function pf_ear_tip_check(pf: PolyFill, pi_ear_tip: PolyIndex): boolean {
  /* localize */
  const coords: vec2[] = pf.coords;
  let pi_curr: PolyIndex;

  let v1: vec2, v2: vec2, v3: vec2;

  // #if defined(USE_CONVEX_SKIP) && !defined(USE_KDTREE)
  //   uint coords_num_concave_checked = 0;
  // #endif

  // #ifdef USE_CONVEX_SKIP

  // #  ifdef USE_CONVEX_SKIP_TEST
  //   /* check if counting is wrong */
  //   {
  //     uint coords_num_concave_test = 0;
  //     PolyIndex *pi_iter = pi_ear_tip;
  //     do {
  //       if (pi_iter->sign != CONVEX) {
  //         coords_num_concave_test += 1;
  //       }
  //     } while ((pi_iter = pi_iter->next) != pi_ear_tip);
  //     BLI_assert(coords_num_concave_test == pf->coords_num_concave);
  //   }
  // #  endif

  //   /* fast-path for circles */
  //   if (pf->coords_num_concave == 0) {
  //     return true;
  //   }
  // #endif

  if (pi_ear_tip.sign == Sign.CONCAVE) {
    return false;
  }

  // #ifdef USE_KDTREE
  //   {
  //     const uint ind[3] = {pi_ear_tip->index, pi_ear_tip->next->index, pi_ear_tip->prev->index};

  //     if (kdtree2d_isect_tri(&pf->kdtree, ind)) {
  //       return false;
  //     }
  //   }
  // #else

  v1 = coords[pi_ear_tip.prev.index];
  v2 = coords[pi_ear_tip.index];
  v3 = coords[pi_ear_tip.next.index];

  /* Check if any point is inside the triangle formed by previous, current and next vertices.
   * Only consider vertices that are not part of this triangle,
   * or else we'll always find one inside. */

  for (pi_curr = pi_ear_tip.next.next; pi_curr !== pi_ear_tip.prev; pi_curr = pi_curr.next) {
    /* Concave vertices can obviously be inside the candidate ear,
     * but so can tangential vertices if they coincide with one of the triangle's vertices. */
    if (pi_curr.sign != Sign.CONVEX) {
      const v: vec2 = coords[pi_curr.index];
      /* Because the polygon has clockwise winding order,
       * the area sign will be positive if the point is strictly inside.
       * It will be 0 on the edge, which we want to include as well. */

      /* NOTE: check (v3, v1) first since it fails _far_ more often than the other 2 checks
       * (those fail equally).
       * It's logical - the chance is low that points exist on the
       * same side as the ear we're clipping off. */
      if (
        span_tri_v2_sign(v3, v1, v) != Sign.CONCAVE &&
        span_tri_v2_sign(v1, v2, v) != Sign.CONCAVE &&
        span_tri_v2_sign(v2, v3, v) != Sign.CONCAVE
      ) {
        return false;
      }

      // #  ifdef USE_CONVEX_SKIP
      //       coords_num_concave_checked += 1;
      //       if (coords_num_concave_checked == pf->coords_num_concave) {
      //         break;
      //       }
      // #  endif
    }
  }
  // #endif; /* USE_KDTREE */

  return true;
}

function pf_ear_tip_cut(pf: PolyFill, pi_ear_tip: PolyIndex): void {
  pf.tris.push([pi_ear_tip.prev.index, pi_ear_tip.index, pi_ear_tip.next.index]);
  pf.tris_num++;

  pf_coord_remove(pf, pi_ear_tip);
}

/**
 * Initializes the #PolyFill structure before tessellating with #polyfill_calc.
 */
function polyfill_prepare(
  pf: PolyFill,
  coords: vec2[],
  coords_num: number,
  coords_sign: number,
  r_tris: vec3[],
  r_indices: PolyIndex[]
): void {
  /* localize */
  const indices: PolyIndex[] = r_indices;

  let i: number;

  /* assign all polyfill members here */
  pf.indices = r_indices;
  pf.coords = coords;
  pf.coords_num = coords_num;

  // #ifdef USE_CONVEX_SKIP
  // pf->coords_num_concave = 0;
  // #endif

  pf.tris = r_tris;
  pf.tris_num = 0;

  if (coords_sign == 0) {
    coords_sign = cross_poly_v2(coords, coords_num) >= 0 ? 1 : -1;
  }

  if (coords_sign == 1) {
    for (i = 0; i < coords_num; i++) {
      indices[i].next = indices[i + 1];
      indices[i].prev = indices[i - 1];
      indices[i].index = i;
    }
  } else {
    /* reversed */
    const n = coords_num - 1;
    for (i = 0; i < coords_num; i++) {
      indices[i].next = indices[i + 1];
      indices[i].prev = indices[i - 1];
      indices[i].index = n - i;
    }
  }
  indices[0].prev = indices[coords_num - 1];
  indices[coords_num - 1].next = indices[0];

  for (i = 0; i < coords_num; i++) {
    const pi: PolyIndex = indices[i];
    pf_coord_sign_calc(pf, pi);
    // #ifdef USE_CONVEX_SKIP
    //     if (pi->sign != CONVEX) {
    //       pf->coords_num_concave += 1;
    //     }
    // #endif
  }
}

function polyfill_calc(pf: PolyFill): void {
  // #ifdef USE_KDTREE
  // #  ifdef USE_CONVEX_SKIP
  // if (pf->coords_num_concave)
  // #  endif
  // {
  // kdtree2d_new(&pf->kdtree, pf->coords_num_concave, pf->coords);
  // kdtree2d_init(&pf->kdtree, pf->coords_num, pf->indices);
  // kdtree2d_balance(&pf->kdtree);
  // kdtree2d_init_mapping(&pf->kdtree);
  // }
  // #endif

  pf_triangulate(pf);
}

export function BLI_polyfill_calc(
  coords: vec2[],
  coords_num: number,
  coords_sign: number,
  r_tris: vec3[]
): void {
  const pf: PolyFill = {
    indices: [],
    coords: [],
    coords_num: 0,
    tris: [],
    tris_num: 0
  };
  const indices: Partial<PolyIndex>[] = new Array(coords_num);
  for (let i = 0, n = indices.length; i < n; ++i) {
    indices[i] = {};
  }

  if (DEBUG_TIME) console.time('polyfill2d');

  polyfill_prepare(pf, coords, coords_num, coords_sign, r_tris, <any>indices);

  polyfill_calc(pf);

  if (DEBUG_TIME) console.timeEnd('polyfill2d');
}
