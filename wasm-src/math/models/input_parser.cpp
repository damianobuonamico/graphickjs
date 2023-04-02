#include "input_parser.h"

#include "corner_detector.h"
#include "path_simplifier.h"
#include "path_fitter.h"
#include "../../editor/settings.h"
#include "../vector.h"

#include "../../utils/console.h"


std::vector<PathBezier> parse_input(const std::vector<PathPoint>& input, std::vector<uint>& corners) {
  const size_t input_size = input.size();
  const float min_dist = 1.0f;

  std::vector<PathPoint> result;
  std::vector<uint> result_corners;

  detect_corners(
    input,
    Settings::corners_radius_min,
    Settings::corners_radius_max,
    Settings::corners_angle_threshold,
    Settings::corners_min_distance,
    Settings::corners_samples_max
  );

  if (input_size < 2) {
    return {};
  }

  PathPoint point{};
  uint last_index = 0;

  for (size_t i = 0; i < corners.size() - 1; i++) {
    uint start = corners[i];
    uint end = corners[i + 1];

    std::vector<uint> simplified = simplify_path(input, start, end, Settings::simplification_threshold);

    uint diff = simplified[1] - start;

    point.position = input[start].position;
    point.pressure = input[start].pressure;

    if (diff > 1) {
      uint half = diff / 2;

      for (uint k = 1; k < half; k++) {
        point.pressure = (point.pressure * half + input[start + k].pressure * (half - k)) / (half + half - k);
      }
    }

    result.push_back(point);
    result_corners.push_back(result.size() - 1);

    for (size_t j = 1; j < simplified.size() - 1; j++) {
      uint prev = simplified[j - 1];
      uint index = simplified[j];
      uint next = simplified[j + 1];
      uint prev_diff = index - prev;
      uint next_diff = next - index;

      point.position = input[index].position;
      point.pressure = input[index].pressure;

      if (prev_diff > 1) {
        uint half = prev_diff / 2;

        for (uint k = 1; k < half; k++) {
          point.pressure = (point.pressure * half + input[index - k].pressure * (half - k)) / (half + half - k);
        }
      }

      if (next_diff > 1) {
        uint half = next_diff / 2;

        for (uint k = 1; k < half; k++) {
          point.pressure = (point.pressure * half + input[index + k].pressure * (half - k)) / (half + half - k);
        }
      }

      last_index = index;
      result.push_back(point);
    }
  }

  uint last_corner = corners[corners.size() - 1];
  uint diff = last_corner - last_index;

  point.position = input[last_corner].position;
  point.pressure = input[last_corner].pressure;

  if (diff > 1) {
    uint half = diff / 2;

    for (uint k = 1; k < half; k++) {
      point.pressure = (point.pressure * half + input[last_corner - k].pressure * (half - k)) / (half + half - k);
    }
  }

  result.push_back(point);
  result_corners.push_back((uint)result.size() - 1);

#if 0
  std::vector<PathBezier> beziers;

  for (size_t i = 0; i < result_corners.size() - 1; i++) {
    uint start = result_corners[i];
    uint end = result_corners[i + 1];

    fit_path(result, start, end, Settings::max_fit_error, beziers);
  }

  return beziers;
#else 
  return {};
#endif
}


// std::vector<PathBezier> parse_input(const std::vector<PathPoint>& input) {
//   const size_t input_size = input.size();
//   const float min_dist = 1.0f;

//   std::vector<PathPoint> result;
//   std::vector<uint> result_corners;

//   std::vector<uint> corners = detect_corners(
//     input,
//     Settings::corners_radius_min,
//     Settings::corners_radius_max,
//     Settings::corners_angle_threshold,
//     Settings::corners_min_distance,
//     Settings::corners_samples_max
//   );

//   if (input_size < 2) {
//     return {};
//   }

//   PathPoint point{};
//   uint last_index = 0;

//   for (size_t i = 0; i < corners.size() - 1; i++) {
//     uint start = corners[i];
//     uint end = corners[i + 1];

//     std::vector<uint> simplified = simplify_path(input, start, end, Settings::simplification_threshold);

//     uint diff = simplified[1] - start;

//     point.position = input[start].position;
//     point.pressure = input[start].pressure;

//     if (diff > 1) {
//       uint half = diff / 2;

//       for (uint k = 1; k < half; k++) {
//         point.pressure = (point.pressure * half + input[start + k].pressure * (half - k)) / (half + half - k);
//       }
//     }

//     result.push_back(point);
//     result_corners.push_back(result.size() - 1);

//     for (size_t j = 1; j < simplified.size() - 1; j++) {
//       uint prev = simplified[j - 1];
//       uint index = simplified[j];
//       uint next = simplified[j + 1];
//       uint prev_diff = index - prev;
//       uint next_diff = next - index;

//       point.position = input[index].position;
//       point.pressure = input[index].pressure;

//       if (prev_diff > 1) {
//         uint half = prev_diff / 2;

//         for (uint k = 1; k < half; k++) {
//           point.pressure = (point.pressure * half + input[index - k].pressure * (half - k)) / (half + half - k);
//         }
//       }

//       if (next_diff > 1) {
//         uint half = next_diff / 2;

//         for (uint k = 1; k < half; k++) {
//           point.pressure = (point.pressure * half + input[index + k].pressure * (half - k)) / (half + half - k);
//         }
//       }

//       last_index = index;
//       result.push_back(point);
//     }
//   }

//   uint last_corner = corners[corners.size() - 1];
//   uint diff = last_corner - last_index;

//   point.position = input[last_corner].position;
//   point.pressure = input[last_corner].pressure;

//   if (diff > 1) {
//     uint half = diff / 2;

//     for (uint k = 1; k < half; k++) {
//       point.pressure = (point.pressure * half + input[last_corner - k].pressure * (half - k)) / (half + half - k);
//     }
//   }

//   result.push_back(point);
//   result_corners.push_back((uint)result.size() - 1);

//   std::vector<PathBezier> beziers;

//   for (size_t i = 0; i < result_corners.size() - 1; i++) {
//     uint start = result_corners[i];
//     uint end = result_corners[i + 1];

//     // std::vector<PathPoint> points;
//     // points.insert(points.begin(), result.begin() + start, result.begin() + end + 1);

//     /*
//       if (Settings::upsample_before_fitting) {
//         const int res = 4;

//         std::vector<PathPoint> points;
//         points.reserve(end - start);

//         for (uint j = start; j <= end; j++) {
//           PathBezier curve{};

//           curve.p0 = result[j - 1].position;
//           curve.p1 = result[j - 1].position + (result[j].position - result[j - 1].position) * 0.33f;
//           curve.p2 = result[j].position - (result[j + 1].position - result[j - 1].position) * 0.33f;
//           curve.p3 = result[j].position;

//           curve.pressure.x = result[j - 1].pressure;
//           curve.pressure.y = result[j].pressure;

//           for (uint k = 0; k < res; k++) {
//             float t = (float)k / (float)res;
//             points.push_back({ bezier(curve.p0, curve.p1, curve.p2, curve.p3, t), lerp(curve.pressure.x, curve.pressure.y, t) });
//           }

//           beziers.push_back(curve);
//         }

//         PathBezier curve{};

//         curve.p0 = result[end - 1].position;
//         curve.p1 = result[end - 1].position + (result[end].position - result[end - 1].position) * 0.33f;
//         curve.p2 = result[end].position;
//         curve.p3 = result[end].position;

//         curve.pressure.x = result[end - 1].pressure;
//         curve.pressure.y = result[end].pressure;

//         beziers.push_back(curve);

//         if (points.size() > 1) {
//           fit_path(points, 0, points.size() - 1, Settings::max_fit_error, beziers);
//         }
//       }
//     */

//     if (!Settings::upsample_before_fitting) {
//       fit_path(result, start, end, Settings::max_fit_error, beziers);
//       continue;
//     }

//     const uint knots_len = end - start + 1;
//     Knot* knots = new Knot[knots_len];

//     // vec2* tangents = new vec2[knots_len * 2];

//     {
//       // vec2* t_step = tangents;

//       for (uint i = 0; i < knots_len; i++) {
//         knots[i].next = knots + i + 1;
//         knots[i].prev = knots + i - 1;

//         // knots[i].heap_node = nullptr;
//         // knots[i].index = i;
//         knots[i].left = vec2{ 0.0f };
//         knots[i].position = result[start + i].position;
//         knots[i].right = vec2{ 0.0f };
//         // knots[i].pressure = result[start + i].pressure;
//         knots[i].pressure = 1.0f;
//         knots[i].can_remove = true;
//         knots[i].is_removed = false;
//         // knots[i].error_sq_next = 0.0f;
//         // knots[i].tan[0] = t_step; t_step++;
//         // knots[i].tan[1] = t_step; t_step++;
//       }

//       // assert((float*)t_step == &tangents[knots_len * 2]);
//     }

//     knots[0].prev = nullptr;
//     knots[knots_len - 1].next = nullptr;

//     /* always keep end-points */
//     knots[0].can_remove = false;
//     knots[knots_len - 1].can_remove = false;

//     {
//       vec2 tan_prev, tan_next;
//       float len_prev, len_next;

//       if (knots_len < 2) {
//         /* NOP, set dummy values */
//         for (uint i = 0; i < knots_len; i++) {
//           Knot* k = &knots[i];

//           zero(k->left);
//           zero(k->right);
//           // zero(*k->tan[0]);
//           // zero(*k->tan[1]);
//           // zero(k->handles);
//         }
//       } else {
//         // len_prev = distance(knots[0].position, knots[1].position);
//         // tan_prev = (knots[0].position - knots[1].position) / len_prev;

//         knots[0].right = 0.666f * (knots[1].position - knots[0].position);

//         // *knots[0].tan[0] = tan_prev;
//         // *knots[0].tan[1] = tan_prev;
//         // knots[0].handles.x = len_prev / 3.0f;
//         // knots[0].handles.y = -len_prev / 3.0f;

//         for (uint i = 1; i < knots_len - 1; i++) {
//           Knot* k_prev = &knots[i - 1];
//           Knot* k = &knots[i];
//           Knot* k_next = &knots[i + 1];

//           k->right = 0.333f * (k_next->position - k_prev->position);
//           k->left = -k->right;

//           // len_next = distance(points[i_curr].position, points[i_next].position);
//           // tan_next = (points[i_curr].position - points[i_next].position) / len_next;

//           // *k->tan[0] = normalize(tan_prev + tan_next);
//           // *k->tan[1] = *k->tan[0];
//           // k->handles.x = len_prev / 3.0f;
//           // k->handles.y = len_next / -3.0f;

//           // tan_prev = tan_next;
//           // len_prev = len_next;
//         }


//         knots[knots_len - 1].left = -0.666f * (knots[knots_len - 1].position - knots[knots_len - 2].position);

//         // *knots[knots_len - 1].tan[0] = tan_next;
//         // *knots[knots_len - 1].tan[1] = tan_next;

//         // knots[knots_len - 1].handles.x = len_next / 3.0f;
//         // knots[knots_len - 1].handles.y = len_next / -3.0f;
//       }
//     }

//     uint knots_len_remaining = knots_len;

//     /* 'curve_incremental_simplify_refit' can be called here, but its very slow
//      * just remove all within the threshold first. */
//     knots_len_remaining = simplify_spline(
//       knots, knots_len, knots_len_remaining,
//       Settings::max_fit_error * Settings::max_fit_error
//     );

// #ifdef USE_KNOT_REFIT
//     knots_len_remaining = curve_incremental_simplify_refit(
//       &pd, knots, knots_len, knots_len_remaining,
//       SQUARE(error_threshold),
//       dims);
// #endif  /* USE_KNOT_REFIT */

//     struct Knot* knots_first = nullptr;

//     {
//       struct Knot* k;

//       for (uint i = 0; i < knots_len; i++) {
//         if (knots[i].is_removed == false) {
//           knots_first = &knots[i];
//           break;
//         }
//       }
//     }

//     {
//       struct Knot* k_prev = knots_first;
//       struct Knot* k = k_prev->next;

//       for (uint i = 1; i < knots_len_remaining; i++, k = k->next) {
//         // const PathPoint* prev = &points[k_prev->index];
//         // const PathPoint* p = &points[k->index];
//         PathBezier bezier{};

//         bezier.p0 = k_prev->position;
//         bezier.p1 = k_prev->position + k_prev->right;
//         bezier.p2 = k->position + k->left;
//         bezier.p3 = k->position;

//         bezier.pressure = { k_prev->pressure, k->pressure };

//         // bezier.p0 = prev->position;
//         // bezier.p1 = prev->position + k_prev->handles.y * (*k_prev->tan[1]);
//         // bezier.p2 = p->position + k->handles.x * (*k->tan[0]);
//         // bezier.p3 = p->position;

//         // bezier.pressure = { prev->pressure, p->pressure };

//         beziers.push_back(bezier);

//         k_prev = k;
//       }
//     }

//     delete[] knots;
//     // delete[] tangents;
//   }

//   return beziers;
// }
