import { vec2, vec3 } from '@/math';

enum Sign {
  CONCAVE = -1,
  TANGENTIAL = 0,
  CONVEX = 1
}

interface PolyFill {
  indices: PolyIndex[] /* vertex aligned */;
  coords: vec2[];
  coords_num: number;
  tris: vec3[];
}

/** Circular double linked-list. */
interface PolyIndex {
  next?: PolyIndex;
  prev?: PolyIndex;
  index: number;
  sign: Sign;
}

function cross_poly_v2(verts: vec2[], nr: number): number {
  let cross: number = 0;
  let co_prev: vec2 = verts[nr - 1];
  let co_curr: vec2;

  for (let a = 0; a < nr; ++a) {
    co_curr = verts[a];
    cross += (co_curr[0] - co_prev[0]) * (co_curr[1] + co_prev[1]);
    co_prev = co_curr;
  }

  return cross;
}
function signum_enum(a: number): Sign {
  if (a === 0) {
    return 0;
  }
  if (a > 0) {
    return 1;
  }

  return -1;
}

function area_tri_signed_v2_alt_2x(v1: vec2, v2: vec2, v3: vec2): number {
  const d2 = vec2.sub(v2, v1);
  const d3 = vec2.sub(v3, v1);
  return d2[0] * d3[1] - d3[0] * d2[1];
}

function span_tri_v2_sign(v1: vec2, v2: vec2, v3: vec2): Sign {
  return signum_enum(area_tri_signed_v2_alt_2x(v3, v2, v1));
}

function pf_coord_sign_calc(pf: PolyFill, pi: PolyIndex) {
  const coords = pf.coords;

  pi.sign = span_tri_v2_sign(coords[pi.prev!.index], coords[pi.index], coords[pi.next!.index]);
}

function polyfill_prepare(pf: PolyFill): void {
  const coords_num = pf.coords_num;
  const coords_sign = cross_poly_v2(pf.coords, coords_num) >= 0 ? 1 : -1;
  const indices = pf.indices;

  let i: number;

  if (coords_sign === 1) {
    for (i = 0; i < coords_num; ++i) {
      indices[i].next = indices[i + 1];
      indices[i].prev = indices[i - 1];
      indices[i].index = i;
    }
  } else {
    /* reversed */
    const n = coords_num - 1;
    for (i = 0; i < coords_num; ++i) {
      indices[i].next = indices[i + 1];
      indices[i].prev = indices[i - 1];
      indices[i].index = n - i;
    }
  }
  indices[0].prev = indices[coords_num - 1];
  indices[coords_num - 1].next = indices[0];

  for (i = 0; i < coords_num; ++i) {
    const pi: PolyIndex = indices[i];
    pf_coord_sign_calc(pf, pi);
  }
}

function pf_coord_remove(pf: PolyFill, pi: PolyIndex): void {
  pi.next!.prev = pi.prev;
  pi.prev!.next = pi.next;

  if (pf.indices[0] === pi) {
    pf.indices.shift();
  }

  pf.coords_num--;
}

function pf_ear_tip_check(pf: PolyFill, pi_ear_tip: PolyIndex): boolean {
  const coords = pf.coords;
  let pi_curr: PolyIndex;

  let v1: vec2, v2: vec2, v3: vec2;

  if (pi_ear_tip.sign === Sign.CONCAVE) {
    return false;
  }

  v1 = coords[pi_ear_tip.prev!.index];
  v2 = coords[pi_ear_tip.index];
  v3 = coords[pi_ear_tip.next!.index];

  /* Check if any point is inside the triangle formed by previous, current and next vertices.
   * Only consider vertices that are not part of this triangle,
   * or else we'll always find one inside. */

  for (pi_curr = pi_ear_tip.next!.next!; pi_curr !== pi_ear_tip.prev; pi_curr = pi_curr.next!) {
    /* Concave vertices can obviously be inside the candidate ear,
     * but so can tangential vertices if they coincide with one of the triangle's vertices. */
    if (pi_curr.sign !== Sign.CONVEX) {
      const v = coords[pi_curr.index];
      /* Because the polygon has clockwise winding order,
       * the area sign will be positive if the point is strictly inside.
       * It will be 0 on the edge, which we want to include as well. */

      /* NOTE: check (v3, v1) first since it fails _far_ more often than the other 2 checks
       * (those fail equally).
       * It's logical - the chance is low that points exist on the
       * same side as the ear we're clipping off. */

      if (
        span_tri_v2_sign(v3, v1, v) !== Sign.CONCAVE &&
        span_tri_v2_sign(v1, v2, v) !== Sign.CONCAVE &&
        span_tri_v2_sign(v2, v3, v) !== Sign.CONCAVE
      ) {
        return false;
      }
    }
  }

  return true;
}

function pf_ear_tip_find(pf: PolyFill, pi_ear_init: PolyIndex, reverse: boolean): PolyIndex {
  /* localize */
  const coords_num = pf.coords_num;
  let pi_ear = pi_ear_init;
  let i = coords_num;

  while (i--) {
    if (pf_ear_tip_check(pf, pi_ear)) {
      return pi_ear;
    }
    pi_ear = reverse ? pi_ear.prev! : pi_ear.next!;
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
    if (pi_ear.sign !== Sign.CONCAVE) {
      return pi_ear;
    }
    pi_ear = pi_ear.next!;
  }

  /* If all vertices are concave, just return the last one. */
  return pi_ear;
}

function pf_ear_tip_cut(pf: PolyFill, pi_ear_tip: PolyIndex): void {
  pf.tris.push([pi_ear_tip.prev!.index, pi_ear_tip.index, pi_ear_tip.next!.index]);
  pf_coord_remove(pf, pi_ear_tip);
}

function pf_triangulate(pf: PolyFill) {
  /* localize */
  let pi_ear: PolyIndex;
  let pi_ear_init: PolyIndex = pf.indices[0];
  let reverse: boolean = false;

  while (pf.coords_num > 3) {
    let pi_prev: PolyIndex, pi_next: PolyIndex;
    let sign_orig_prev: Sign, sign_orig_next: Sign;

    pi_ear = pf_ear_tip_find(pf, pi_ear_init, reverse);

    pi_prev = pi_ear.prev!;
    pi_next = pi_ear.next!;

    pf_ear_tip_cut(pf, pi_ear);

    /* The type of the two vertices adjacent to the clipped vertex may have changed. */
    sign_orig_prev = pi_prev.sign;
    sign_orig_next = pi_next.sign;

    if (sign_orig_prev !== Sign.CONVEX) {
      pf_coord_sign_calc(pf, pi_prev);
    }

    if (sign_orig_next !== Sign.CONVEX) {
      pf_coord_sign_calc(pf, pi_next);
    }

    pi_ear_init = reverse ? pi_prev.prev! : pi_next.next!;

    if (pi_ear_init.sign !== Sign.CONVEX) {
      /* take the extra step since this ear isn't a good candidate */
      pi_ear_init = reverse ? pi_ear_init.prev! : pi_ear_init.next!;
      reverse = !reverse;
    }
  }

  if (pf.coords_num === 3) {
    const tri = vec3.create();
    pi_ear = pf.indices[0];
    tri[0] = pi_ear.index;
    pi_ear = pi_ear.next!;
    tri[1] = pi_ear.index;
    pi_ear = pi_ear.next!;
    tri[2] = pi_ear.index;
  }
}

export function BLI_polyfill_calc(coords: vec2[]): vec3[] {
  const coords_num = coords.length;
  const indices: PolyIndex[] = new Array(coords_num)
    .fill(undefined)
    .map((u, index) => ({ index, sign: 0 }));

  const pf: PolyFill = {
    indices,
    coords,
    coords_num,
    tris: []
  };

  polyfill_prepare(pf);
  pf_triangulate(pf);

  return pf.tris;
}
