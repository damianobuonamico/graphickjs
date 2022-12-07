import { vec2, vec3 } from '@/math';
import { GEOMETRY_MAX_ERROR } from '@/utils/constants';
import { BLI_polyfill_calc } from './polyfill2dBlend';

/* Grease-Pencil Annotations - 'Stroke Point'
 * -> Coordinates of point on stroke, in proportions of window size
 *    This assumes that the bottom-left corner is (0,0)
 */
export interface bGPDspoint {
  /** Coordinates of point. */
  x: number;
  y: number;
  z: number;

  /** Pressure of input device (from 0 to 1) at this point. */
  pressure: number;

  /** Seconds since start of stroke. */
  time: number;

  /** Factor of uv along the stroke. */
  uv_fac: number;
  /** Uv rotation for dot mode. */
  uv_rot: number;
  /** Uv for fill mode */
  uv_fill: vec2;

  /** Vertex Color RGBA (A=mix factor). */
  vert_color: vec4;
}

/* Grease-Pencil Annotations - 'Triangle'
 * -> A triangle contains the index of three vertices for filling the stroke
 *    This is only used if high quality fill is enabled
 */
interface bGPDtriangle {
  /* indices for tessellated triangle used for GP Fill */
  verts: vec3;
}

/* Grease-Pencil Annotations - 'Stroke'
 * -> A stroke represents a (simplified version) of the curve
 *    drawn by the user in one 'mouse-down'->'mouse-up' operation
 */
export interface bGPDstroke {
  next: bGPDstroke | null;
  prev: bGPDstroke | null;

  /** Array of data-points for stroke. */
  points: bGPDspoint[] | null;
  /** Tessellated triangles for GP Fill. */
  triangles: vec3[] | null;
  /** Number of data-points in array. */
  totpoints: number;
  /** Number of triangles in array. */
  tot_triangles: number;

  /** Thickness of stroke. */
  thickness: number;
  /** Various settings about this stroke. */
  flag: GPDstroke_Flag;
  _pad: vec2;

  /** Init time of stroke. */
  inittime: number;

  /** Material index. */
  mat_nr: number;
  /** Caps mode for each stroke extreme */
  caps: vec2;

  /** gradient control along y for color */
  hardeness: number;
  /** factor xy of shape for dots gradients */
  aspect_ratio: vec2;

  /** Factor of opacity for Fill color (used by opacity modifier). */
  fill_opacity_fac: number;

  /** Min of the bound box used to speedup painting operators. */
  boundbox_min: vec2;
  /** Max of the bound box used to speedup painting operators. */
  boundbox_max: vec2;

  /** UV rotation */
  uv_rotation: number;
  /** UV translation (X and Y axis) */
  uv_translation: vec2;
  uv_scale: number;

  /** Stroke selection index. */
  select_index: number;
  _pad4: vec4;

  /** Vertex Color for Fill (one for all stroke, A=mix factor). */
  vert_color_fill: vec4;

  /* NOTE: When adding new members, make sure to add them to BKE_gpencil_stroke_copy_settings as
   * well! */
}

/** #bGPDstroke.caps */
enum GPDstroke_Caps {
  /* type of extreme */
  GP_STROKE_CAP_ROUND = 0,
  GP_STROKE_CAP_FLAT = 1,

  /* Keep last. */
  GP_STROKE_CAP_MAX
}

/* Arrows ----------------------- */

/** #bGPDataRuntime.arrowstyle */
enum eGPDstroke_Arrowstyle {
  GP_STROKE_ARROWSTYLE_NONE = 0,
  GP_STROKE_ARROWSTYLE_SEGMENT = 2,
  GP_STROKE_ARROWSTYLE_OPEN = 3,
  GP_STROKE_ARROWSTYLE_CLOSED = 4,
  GP_STROKE_ARROWSTYLE_SQUARE = 6
}

/** #bGPDstroke.flag */
enum GPDstroke_Flag {
  /* stroke is in 3d-space */
  GP_STROKE_3DSPACE = 1 << 0,
  /* stroke is in 2d-space */
  GP_STROKE_2DSPACE = 1 << 1,
  /* stroke is in 2d-space (but with special 'image' scaling) */
  GP_STROKE_2DIMAGE = 1 << 2,
  /* stroke is selected */
  GP_STROKE_SELECT = 1 << 3,
  /* Flag used to indicate that stroke is closed and draw edge between last and first point */
  GP_STROKE_CYCLIC = 1 << 7,
  /* Flag used to indicate that stroke is used for fill close and must use
   * fill color for stroke and no fill area */
  GP_STROKE_NOFILL = 1 << 8,
  /* Flag to indicated that the editcurve has been changed and the stroke needs to be updated with
   * the curve data */
  GP_STROKE_NEEDS_CURVE_UPDATE = 1 << 9,
  /* Flag to indicate that a stroke is used only for help, and will not affect rendering or fill */
  GP_STROKE_HELP = 1 << 10,
  /* Flag to indicate that a extend stroke collide (fill tool)  */
  GP_STROKE_COLLIDE = 1 << 11,
  /* only for use with stroke-buffer (while drawing arrows) */
  GP_STROKE_USE_ARROW_START = 1 << 12,
  /* only for use with stroke-buffer (while drawing arrows) */
  GP_STROKE_USE_ARROW_END = 1 << 13,
  /* Tag for update geometry */
  GP_STROKE_TAG = 1 << 14,
  /* only for use with stroke-buffer (while drawing eraser) */
  GP_STROKE_ERASER = 1 << 15
}

export function BKE_gpencil_stroke_new(
  mat_idx: number,
  totpoints: number,
  thickness: number
): bGPDstroke {
  const gps: bGPDstroke = {
    next: null,
    prev: null,
    points: totpoints > 0 ? new Array(totpoints) : null,
    triangles: null,
    totpoints: totpoints,
    tot_triangles: 0,
    thickness: thickness,
    flag: GPDstroke_Flag.GP_STROKE_2DSPACE,
    _pad: [0, 0],
    inittime: 0,
    mat_nr: mat_idx,
    caps: [GPDstroke_Caps.GP_STROKE_CAP_ROUND, GPDstroke_Caps.GP_STROKE_CAP_ROUND],
    hardeness: 1,
    aspect_ratio: [1, 1],
    fill_opacity_fac: 1,
    boundbox_min: [0, 0],
    boundbox_max: [0, 0],
    uv_rotation: 0,
    uv_translation: [0, 0],
    uv_scale: 1,
    select_index: 0,
    _pad4: [0, 0, 0, 0],
    vert_color_fill: [0, 0, 0, 1]
  };

  return gps;
}

function BKE_gpencil_stroke_2d_flat(
  points: bGPDspoint[],
  totpoints: number,
  points2d: vec2[],
  r_direction: number
): number {
  if (totpoints < 2) return r_direction;

  const pt0: bGPDspoint = points[0];
  const pt1: bGPDspoint = points[1];
  const pt3: bGPDspoint = points[Math.round(totpoints * 0.75)];

  const locx: vec3 = vec3.create();
  const locy: vec3 = vec3.create();
  const loc3: vec3 = vec3.create();
  const normal: vec3 = vec3.create();

  /* local X axis (p0 -> p1) */
  vec3.sub([pt1.x, pt1.y, pt1.z], [pt0.x, pt0.y, pt0.z], locx);

  /* point vector at 3/4 */
  let v3: vec3 = vec3.create();
  if (totpoints === 2) {
    vec3.mulS([pt3.x, pt3.y, pt3.z], 0.001, v3);
  } else {
    vec3.copy(v3, [pt3.x, pt3.y, pt3.z]);
  }

  vec3.sub(v3, [pt0.x, pt0.y, pt0.z], loc3);

  /* vector orthogonal to polygon plane */
  vec3.cross(locx, loc3, normal);

  /* local Y axis (cross to normal/x axis) */
  vec3.cross(normal, locx, locy);

  /* Normalize vectors */
  vec3.normalize(locx, locx);
  vec3.normalize(locy, locy);

  /* Calculate last point first. */
  const pt_last: bGPDspoint = points[totpoints - 1];
  const tmp = vec3.sub([pt_last.x, pt_last.y, pt_last.z], [pt0.x, pt0.y, pt0.z]);

  points2d[totpoints - 1][0] = vec3.dot(tmp, locx);
  points2d[totpoints - 1][1] = vec3.dot(tmp, locy);

  /* Calculate the scalar cross product of the 2d points. */
  let cross: number = 0;
  let co_curr: vec2;
  let co_prev: vec2 = points2d[totpoints - 1];

  /* Get all points in local space */
  for (let i = 0; i < totpoints - 1; i++) {
    const pt: bGPDspoint = points[i];

    /* Get local space using first point as origin */
    let loc: vec3 = vec3.sub([pt.x, pt.y, pt.z], [pt0.x, pt0.y, pt0.z]);

    points2d[i][0] = vec3.dot(loc, locx);
    points2d[i][1] = vec3.dot(loc, locy);

    /* Calculate cross product. */
    co_curr = points2d[i];
    cross += (co_curr[0] - co_prev[0]) * (co_curr[1] + co_prev[1]);
    co_prev = points2d[i];
  }

  /* Concave (-1), Convex (1) */
  return cross >= 0 ? 1 : -1;
}

function generate_arc_from_point_to_point(
  list: vec2[],
  from: vec2,
  to: vec2,
  from_index: number,
  to_index: number,
  center_pt: vec2,
  subdivisions: number,
  clockwise: boolean
): number {
  const vec_from = vec2.sub(from, center_pt);
  const vec_to = vec2.sub(to, center_pt);
  if (vec2.isZero(vec_from) || vec2.isZero(vec_to)) {
    return 0;
  }

  const dot = vec2.dot(vec_from, vec_to);
  const det = vec2.cross(vec_from, vec_to);
  const angle = clockwise ? Math.PI - Math.atan2(-det, -dot) : Math.atan2(-det, -dot) + Math.PI;

  /* Number of points is 2^(n+1) + 1 on half a circle (n=subdivisions)
   * so we multiply by (angle / pi) to get the right amount of
   * points to insert. */
  const num_points: number = Math.abs(((1 << (subdivisions + 1)) - 1) * (angle / Math.PI));
  if (num_points > 0) {
    const angle_incr: number = angle / num_points;

    const vec_p = vec2.create();
    const vec_t = vec2.create();
    let tmp_angle: number;
    let last_point_index: number;
    if (clockwise) {
      last_point_index = to_index;
      vec2.copy(vec_t, vec_to);
    } else {
      last_point_index = from_index;
      vec2.copy(vec_t, vec_from);
    }

    for (let i = 0; i < num_points - 1; ++i) {
      tmp_angle = (i + 1) * angle_incr;

      vec2.rotateOrigin(vec_t, tmp_angle, vec_p);
      vec2.add(vec_p, center_pt, vec_p);

      const new_point: vec2 = vec2.clone(vec_p);
      if (clockwise) {
        // TODO: Maybe optimize with linked list
        list.splice(last_point_index - 1, 0, new_point);
      } else {
        list.splice(last_point_index, 0, new_point);
        last_point_index++;
      }
    }

    return num_points - 1;
  }

  return 0;
}

function generate_semi_circle_from_point_to_point(
  list: vec2[],
  from: vec2,
  to: vec2,
  fromIndex: number,
  subdivisions: number
): number {
  const num_points: number = (1 << (subdivisions + 1)) + 1;
  const center_pt: vec2 = vec2.lerp(from, to, 0.5);

  const vec_center: vec2 = vec2.sub(from, center_pt);
  if (vec2.isZero(vec_center)) {
    return 0;
  }

  const vec_p = vec2.create();
  const angle_incr: number = Math.PI / (num_points - 1);

  let last_point_index: number = fromIndex;
  for (let i = 1; i < num_points; ++i) {
    const angle = i * angle_incr;

    /* Rotate vector around point to get perimeter points. */
    vec2.rotateOrigin(vec_center, angle, vec_p);
    vec2.add(vec_p, center_pt, vec_p);

    const new_point: vec2 = vec2.clone(vec_p);
    list.splice(last_point_index, 0, new_point);
    last_point_index++;
  }

  return num_points - 1;
}

function generate_perimeter_cap(
  point: vec2,
  other_point: vec2,
  radius: number,
  list: vec2[],
  subdivisions: number,
  cap_type: GPDstroke_Caps
): number {
  const cap_vec: vec2 = vec2.sub(other_point, point);
  vec2.normalize(cap_vec, cap_vec);

  const cap_nvec = vec2.create();
  if (vec2.isZero(cap_vec)) {
    cap_nvec[0] = 0;
    cap_nvec[1] = radius;
  } else {
    cap_nvec[0] = -cap_vec[1];
    cap_nvec[1] = cap_vec[0];
    vec2.mulS(cap_nvec, radius, cap_nvec);
  }
  const cap_nvec_inv: vec2 = vec2.neg(cap_nvec);

  const vec_perimeter: vec2 = vec2.add(point, cap_nvec);

  const vec_perimeter_inv: vec2 = vec2.add(point, cap_nvec_inv);

  const p_pt: vec2 = vec2.clone(vec_perimeter);
  const p_pt_inv: vec2 = vec2.clone(vec_perimeter_inv);

  list.push(p_pt);
  list.push(p_pt_inv);

  let num_points: number = 0;
  if (cap_type === GPDstroke_Caps.GP_STROKE_CAP_ROUND) {
    num_points += generate_semi_circle_from_point_to_point(
      list,
      p_pt,
      p_pt_inv,
      list.length - 2,
      subdivisions
    );
  }

  return num_points + 2;
}

export function gpencil_stroke_perimeter_ex(
  points: vec3[],
  thickness: number,
  subdivisions: number,
  caps: [GPDstroke_Caps, GPDstroke_Caps]
): vec2[] {
  const totpoints = points.length;
  const thickness_chg = 0;

  /* sanity check */
  if (totpoints < 1) return [];

  // TODO: Check
  const defaultpixsize = 1; /*1000 / pixfactor*/
  const ovr_radius = thickness_chg / defaultpixsize / 2;
  let stroke_radius = thickness / defaultpixsize / 2;
  stroke_radius = Math.max(stroke_radius - ovr_radius, 0);

  const perimeter_right_side: vec2[] = [];
  const perimeter_left_side: vec2[] = [];
  let num_perimeter_points: number = 0;

  const first: vec3 = points[0];
  const last: vec3 = points[totpoints - 1];

  const first_radius: number = stroke_radius * first[2];
  const last_radius: number = stroke_radius * last[2];

  let first_next: vec3;
  let last_prev: vec3;
  if (totpoints > 1) {
    first_next = points[1];
    last_prev = points[totpoints - 2];
  } else {
    first_next = first;
    last_prev = last;
  }

  const first_pt: vec2 = [first[0], first[1]];
  const last_pt: vec2 = [last[0], last[1]];
  const first_next_pt: vec2 = [first_next[0], first_next[1]];
  const last_prev_pt: vec2 = [last_prev[0], last_prev[1]];

  /* Edge-case if single point. */
  if (totpoints == 1) {
    first_next_pt[0] += 1;
    last_prev_pt[0] -= 1;
  }

  /* Generate points for start cap. */
  num_perimeter_points += generate_perimeter_cap(
    first_pt,
    first_next_pt,
    first_radius,
    perimeter_right_side,
    subdivisions,
    caps[0]
  );

  /* Generate perimeter points. */
  let curr_pt = vec2.create(),
    next_pt = vec2.create(),
    prev_pt = vec2.create();
  let vec_next = vec2.create(),
    vec_prev = vec2.create();
  let nvec_next = vec2.create(),
    nvec_prev = vec2.create();
  let nvec_next_pt = vec2.create(),
    nvec_prev_pt = vec2.create();
  let vec_tangent = vec2.create();

  let vec_miter_left = vec2.create(),
    vec_miter_right = vec2.create();
  let miter_left_pt = vec2.create(),
    miter_right_pt = vec2.create();

  for (let i = 1; i < totpoints - 1; ++i) {
    const curr: vec3 = points[i];
    const prev: vec3 = points[i - 1];
    const next: vec3 = points[i + 1];
    const radius: number = stroke_radius * curr[2];

    vec2.set(curr_pt, curr[0], curr[1]);
    vec2.set(next_pt, next[0], next[1]);
    vec2.set(prev_pt, prev[0], prev[1]);

    vec2.sub(curr_pt, prev_pt, vec_prev);
    vec2.sub(next_pt, curr_pt, vec_next);
    const prev_length: number = vec2.len(vec_prev);
    const next_length: number = vec2.len(vec_next);

    if (vec2.isZero(vec2.normalize(vec_prev, vec_prev))) {
      vec_prev[0] = 1;
      vec_prev[1] = 0;
    }
    if (vec2.isZero(vec2.normalize(vec_next, vec_next))) {
      vec_next[0] = 1;
      vec_next[1] = 0;
    }

    nvec_prev[0] = -vec_prev[1];
    nvec_prev[1] = vec_prev[0];

    nvec_next[0] = -vec_next[1];
    nvec_next[1] = vec_next[0];

    vec2.add(vec_prev, vec_next, vec_tangent);
    if (vec2.isZero(vec2.normalize(vec_tangent, vec_tangent))) {
      vec2.copy(vec_tangent, nvec_prev);
    }

    vec_miter_left[0] = -vec_tangent[1];
    vec_miter_left[1] = vec_tangent[0];

    /* calculate miter length */
    let an1: number = vec2.dot(vec_miter_left, nvec_prev);
    if (an1 === 0) {
      an1 = 1;
    }
    let miter_length = radius / an1;
    if (miter_length <= 0) {
      miter_length = 0.01;
    }

    vec2.normalizeLength(vec_miter_left, miter_length, vec_miter_left);

    vec2.copy(vec_miter_right, vec_miter_left);
    vec2.neg(vec_miter_right, vec_miter_right);

    const angle: number = vec2.dot(vec_next, nvec_prev);
    /* Add two points if angle is close to being straight. */
    if (Math.abs(angle) < 0.0001) {
      vec2.normalizeLength(nvec_prev, radius, nvec_prev);
      vec2.normalizeLength(nvec_next, radius, nvec_next);

      vec2.copy(nvec_prev_pt, curr_pt);
      vec2.add(nvec_prev_pt, nvec_prev, nvec_prev_pt);

      vec2.copy(nvec_next_pt, curr_pt);
      vec2.neg(nvec_next, nvec_next);
      vec2.add(nvec_next_pt, nvec_next, nvec_next_pt);

      const normal_prev: vec2 = vec2.clone(nvec_prev_pt);
      const normal_next: vec2 = vec2.clone(nvec_next_pt);

      perimeter_left_side.push(normal_prev);
      perimeter_right_side.push(normal_next);
      num_perimeter_points += 2;
    } else {
      /* bend to the left */
      if (angle < 0) {
        vec2.normalizeLength(nvec_prev, radius, nvec_prev);
        vec2.normalizeLength(nvec_next, radius, nvec_next);

        vec2.copy(nvec_prev_pt, curr_pt);
        vec2.add(nvec_prev_pt, nvec_prev, nvec_prev_pt);

        vec2.copy(nvec_next_pt, curr_pt);
        vec2.add(nvec_next_pt, nvec_next, nvec_next_pt);

        const normal_prev: vec2 = vec2.clone(nvec_prev_pt);
        const normal_next: vec2 = vec2.clone(nvec_next_pt);

        perimeter_left_side.push(normal_prev);
        perimeter_left_side.push(normal_next);
        num_perimeter_points += 2;

        num_perimeter_points += generate_arc_from_point_to_point(
          perimeter_left_side,
          normal_prev,
          normal_next,
          num_perimeter_points - 2,
          num_perimeter_points - 1,
          curr_pt,
          subdivisions,
          true
        );

        if (miter_length < prev_length && miter_length < next_length) {
          vec2.copy(miter_right_pt, curr_pt);
          vec2.add(miter_right_pt, vec_miter_right, miter_right_pt);
        } else {
          vec2.copy(miter_right_pt, curr_pt);
          vec2.neg(nvec_next, nvec_next);
          vec2.add(miter_right_pt, nvec_next, miter_right_pt);
        }

        const miter_right: vec2 = vec2.clone(miter_right_pt);
        perimeter_right_side.push(miter_right);
        num_perimeter_points++;
      } else {
        vec2.normalizeLength(nvec_prev, -radius, nvec_prev);
        vec2.normalizeLength(nvec_next, -radius, nvec_next);

        vec2.copy(nvec_prev_pt, curr_pt);
        vec2.add(nvec_prev_pt, nvec_prev, nvec_prev_pt);

        vec2.copy(nvec_next_pt, curr_pt);
        vec2.add(nvec_next_pt, nvec_next, nvec_next_pt);

        const normal_prev: vec2 = vec2.clone(nvec_prev_pt);
        const normal_next: vec2 = vec2.clone(nvec_next_pt);

        perimeter_right_side.push(normal_prev);
        perimeter_right_side.push(normal_next);
        num_perimeter_points += 2;

        num_perimeter_points += generate_arc_from_point_to_point(
          perimeter_right_side,
          normal_prev,
          normal_next,
          num_perimeter_points - 2,
          num_perimeter_points - 1,
          curr_pt,
          subdivisions,
          false
        );

        if (miter_length < prev_length && miter_length < next_length) {
          vec2.copy(miter_left_pt, curr_pt);
          vec2.add(miter_left_pt, vec_miter_left, miter_left_pt);
        } else {
          vec2.copy(miter_left_pt, curr_pt);
          vec2.neg(nvec_prev, nvec_prev);
          vec2.add(miter_left_pt, nvec_prev, miter_left_pt);
        }

        const miter_left: vec2 = vec2.clone(miter_left_pt);
        perimeter_left_side.push(miter_left);
        num_perimeter_points++;
      }
    }
  }

  /* generate points for end cap */
  num_perimeter_points += generate_perimeter_cap(
    last_pt,
    last_prev_pt,
    last_radius,
    perimeter_right_side,
    subdivisions,
    caps[1]
  );

  /* merge both sides to one list */
  perimeter_right_side.reverse();
  const perimeter_list: vec2[] = perimeter_left_side.concat(perimeter_right_side);

  /* close by creating a point close to the first (make a small gap) */
  const close_pt: vec2 = vec2.lerp(
    perimeter_list[perimeter_list.length - 1],
    perimeter_list[0],
    0.99
  );

  if (vec2.equals(close_pt, perimeter_list[0], GEOMETRY_MAX_ERROR) === false) {
    const close_p_pt: vec2 = vec2.clone(close_pt);
    perimeter_list.push(close_p_pt);
    num_perimeter_points++;
  }

  return perimeter_list;
}

export function BKE_gpencil_stroke_fill_triangulate(gps: bGPDstroke): vec2[] {
  if (gps.totpoints < 3) return [];

  /* allocate memory for temporary areas */
  gps.tot_triangles = gps.totpoints - 2;
  const tmp_triangles: vec3[] = new Array(gps.tot_triangles);
  const points2d: vec2[] = new Array(gps.totpoints);
  const uv: vec2[] = new Array(gps.totpoints);

  for (let i = 0; i < gps.tot_triangles; i++) {
    tmp_triangles[i] = [0, 0, 0];
    points2d[i] = [0, 0];
  }

  points2d[gps.totpoints - 2] = [0, 0];
  points2d[gps.totpoints - 1] = [0, 0];

  let direction = 0;

  /* convert to 2d and triangulate */
  direction = BKE_gpencil_stroke_2d_flat(gps.points!, gps.totpoints, points2d, direction);
  BLI_polyfill_calc(points2d, gps.totpoints, direction, tmp_triangles);

  // /* calc texture coordinates automatically */
  // float minv[2];
  // float maxv[2];
  // /* first needs bounding box data */
  // ARRAY_SET_ITEMS(minv, -1.0f, -1.0f);
  // ARRAY_SET_ITEMS(maxv, 1.0f, 1.0f);

  // /* calc uv data */
  // gpencil_calc_stroke_fill_uv(points2d, gps, minv, maxv, uv);

  /* Save triangulation data. */
  if (gps.tot_triangles > 0) {
    gps.triangles = tmp_triangles;

    //   /* Copy UVs to bGPDspoint. */
    //   for (int i = 0; i < gps->totpoints; i++) {
    //     copy_v2_v2(gps->points[i].uv_fill, uv[i]);
    //   }
  } else {
    gps.triangles = null;
  }

  return points2d;
}
