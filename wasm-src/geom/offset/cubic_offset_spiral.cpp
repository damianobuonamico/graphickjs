// /**
//  * @file geom/offset/cubic_offset.cpp
//  * @brief Implementation of cubic offsetting.
//  *
//  * The following implementation is an adaptation of the algorithm in
//  * <https://raphlinus.github.io/curves/2022/09/09/parallel-beziers.html>.
//  */

// #include "offset.h"

// #include "../../math/vector.h"

// #include "../curve_ops.h"
// #include "../quadratic_bezier.h"

// #include "../../../renderer/renderer.h"

// namespace graphick::geom {

// const double EPS = 1e-12;
// const double EPS_M = 2.22045e-16;

// const double GAUSS_LEGENDRE_COEFFS_32[] = {
//   0.0965400885147278, -0.0483076656877383, 0.0965400885147278, 0.0483076656877383,  0.0956387200792749, -0.1444719615827965,
//   0.0956387200792749, 0.1444719615827965,  0.0938443990808046, -0.2392873622521371, 0.0938443990808046, 0.2392873622521371,
//   0.0911738786957639, -0.3318686022821277, 0.0911738786957639, 0.3318686022821277,  0.0876520930044038, -0.4213512761306353,
//   0.0876520930044038, 0.4213512761306353,  0.0833119242269467, -0.5068999089322294, 0.0833119242269467, 0.5068999089322294,
//   0.0781938957870703, -0.5877157572407623, 0.0781938957870703, 0.5877157572407623,  0.0723457941088485, -0.6630442669302152,
//   0.0723457941088485, 0.6630442669302152,  0.0658222227763618, -0.7321821187402897, 0.0658222227763618, 0.7321821187402897,
//   0.0586840934785355, -0.7944837959679424, 0.0586840934785355, 0.7944837959679424,  0.0509980592623762, -0.8493676137325700,
//   0.0509980592623762, 0.8493676137325700,  0.0428358980222267, -0.8963211557660521, 0.0428358980222267, 0.8963211557660521,
//   0.0342738629130214, -0.9349060759377397, 0.0342738629130214, 0.9349060759377397,  0.0253920653092621, -0.9647622555875064,
//   0.0253920653092621, 0.9647622555875064,  0.0162743947309057, -0.9856115115452684, 0.0162743947309057, 0.9856115115452684,
//   0.0070186100094701, -0.9972638618494816, 0.0070186100094701, 0.9972638618494816,
// };

// inline static double eps_rel(const double raw, const double a) { return a == 0.0 ? std::abs(raw) : std::abs((raw - a) / a); }

// static double depressed_cubic_dominant(const double g, const double h) {
//   const double q = (-1. / 3) * g;
//   const double r = 0.5 * h;

//   double phi_0;
//   std::optional<double> k = std::nullopt;

//   if (std::abs(q) >= 1e102 || std::abs(r) >= 1e164) {
//     if (std::abs(q) < std::abs(r)) {
//       k = 1 - q * (q / r) * (q / r);
//     } else {
//       k = math::sign(q) * ((r / q) * (r / q) / q - 1);
//     }
//   }

//   if (k.has_value() && r == 0) {
//     if (g > 0) {
//       phi_0 = 0;
//     } else {
//       phi_0 = std::sqrt(-g);
//     }
//   } else if (k.has_value() ? k.value() < 0 : r * r < q * q * q) {
//     const double t = k.has_value() ? r / q / std::sqrt(q) : r / std::sqrt(q * q * q);
//     phi_0 = -2 * std::sqrt(q) * std::copysign(std::cos(std::acos(std::abs(t)) * (1. / 3)), t);
//   } else {
//     double a;
//     if (k.has_value()) {
//       if (std::abs(q) < std::abs(r)) {
//         a = -r * (1 + std::sqrt(k.value()));
//       } else {
//         a = -r - std::copysign(std::sqrt(std::abs(q)) * q * std::sqrt(k.value()), r);
//       }
//     } else {
//       a = std::cbrt(-r - std::copysign(std::sqrt(r * r - q * q * q), r));
//     }

//     const double b = a == 0 ? 0 : q / a;
//     phi_0 = a + b;
//   }

//   double x = phi_0;
//   double f = (x * x + g) * x + h;

//   if (std::abs(f) < EPS_M * std::max({x * x * x, g * x, h})) {
//     return x;
//   }

//   for (int i = 0; i < 8; i++) {
//     const double delt_f = 3 * x * x + g;

//     if (delt_f == 0) {
//       break;
//     }

//     const double new_x = x - f / delt_f;
//     const double new_f = (new_x * new_x + g) * new_x + h;

//     if (new_f == 0) {
//       return new_x;
//     }
//     if (std::abs(new_f) >= std::abs(f)) {
//       break;
//     }

//     x = new_x;
//     f = new_f;
//   }

//   return x;
// }

// static std::optional<std::vector<double>> factor_quartic_inner(
//   const double a,
//   const double b,
//   const double c,
//   const double d,
//   const bool rescale
// ) {
//   const auto calc_eps_q = [&](const double a1, const double b1, const double a2, const double b2) {
//     const double eps_a = eps_rel(a1 + a2, a);
//     const double eps_b = eps_rel(b1 + a1 * a2 + b2, b);
//     const double eps_c = eps_rel(b1 * a2 + a1 * b2, c);

//     return eps_a + eps_b + eps_c;
//   };

//   const auto calc_eps_t = [&](const double a1, const double b1, const double a2, const double b2) {
//     return calc_eps_q(a1, b1, a2, b2) + eps_rel(b1 * b2, d);
//   };

//   const double disc = 9 * a * a - 24 * b;
//   const double s = disc >= 0 ? -2 * b / (3 * a + std::copysign(std::sqrt(disc), a)) : -0.25 * a;
//   const double a_prime = a + 4 * s;
//   const double b_prime = b + 3 * s * (a + 2 * s);
//   const double c_prime = c + s * (2 * b + s * (3 * a + 4 * s));
//   const double d_prime = d + s * (c + s * (b + s * (a + s)));

//   double g_prime = 0.0;
//   double h_prime = 0.0;

//   const double K_C = 3.49e102;

//   if (rescale) {
//     const double a_prime_s = a_prime / K_C;
//     const double b_prime_s = b_prime / K_C;
//     const double c_prime_s = c_prime / K_C;
//     const double d_prime_s = d_prime / K_C;

//     g_prime = a_prime_s * c_prime_s - (4 / K_C) * d_prime_s - (1. / 3) * b_prime_s * b_prime_s;
//     h_prime = (a_prime_s * c_prime_s - (8 / K_C) * d_prime_s - (2. / 9) * b_prime_s * b_prime_s) * (1. / 3) * b_prime_s -
//       c_prime_s * (c_prime_s / K_C) - a_prime_s * a_prime_s * d_prime_s;
//   } else {
//     g_prime = a_prime * c_prime - 4 * d_prime - (1. / 3) * b_prime * b_prime;
//     h_prime = (a_prime * c_prime + 8 * d_prime - (2. / 9) * b_prime * b_prime) * (1. / 3) * b_prime - c_prime * c_prime -
//       a_prime * a_prime * d_prime;
//   }

//   if (!std::isfinite(g_prime) && std::isfinite(h_prime)) {
//     return std::nullopt;
//   }

//   double phi = depressed_cubic_dominant(g_prime, h_prime);

//   if (rescale) {
//     phi *= K_C;
//   }

//   const double l_1 = a * 0.5;
//   const double l_3 = (1. / 6) * b + 0.5 * phi;
//   const double delt_2 = c - a * l_3;
//   const double d_2_cand_1 = (2. / 3) * b - phi - l_1 * l_1;
//   const double l_2_cand_1 = 0.5 * delt_2 / d_2_cand_1;
//   const double l_2_cand_2 = 2 * (d - l_3 * l_3) / delt_2;
//   const double d_2_cand_2 = 0.5 * delt_2 / l_2_cand_2;

//   double d_2_best = 0;
//   double l_2_best = 0;
//   double eps_l_best = 0;

//   for (int i = 0; i < 3; i++) {
//     const double d_2 = i == 1 ? d_2_cand_2 : d_2_cand_1;
//     const double l_2 = i == 0 ? l_2_cand_1 : l_2_cand_2;
//     const double eps_0 = eps_rel(d_2 + l_1 * l_1 + 2 * l_3, b);
//     const double eps_1 = eps_rel(2 * (d_2 * l_2 + l_1 * l_3), c);
//     const double eps_2 = eps_rel(d_2 * l_2 * l_2 + l_3 * l_3, d);
//     const double eps_l = eps_0 + eps_1 + eps_2;

//     if (i == 0 || eps_l < eps_l_best) {
//       d_2_best = d_2;
//       l_2_best = l_2;
//       eps_l_best = eps_l;
//     }
//   }

//   const double d_2 = d_2_best;
//   const double l_2 = l_2_best;

//   double alpha_1 = 0;
//   double beta_1 = 0;
//   double alpha_2 = 0;
//   double beta_2 = 0;

//   if (d_2 < 0.0) {
//     const double sq = std::sqrt(-d_2);

//     alpha_1 = l_1 + sq;
//     beta_1 = l_3 + sq * l_2;
//     alpha_2 = l_1 - sq;
//     beta_2 = l_3 - sq * l_2;

//     if (std::abs(beta_2) < std::abs(beta_1)) {
//       beta_2 = d / beta_1;
//     } else if (std::abs(beta_2) > std::abs(beta_1)) {
//       beta_1 = d / beta_2;
//     }

//     if (std::abs(alpha_1) != std::abs(alpha_2)) {
//       std::vector<double> a1_cands;
//       std::vector<double> a2_cands;

//       if (std::abs(alpha_1) < std::abs(alpha_2)) {
//         const double a1_cand_1 = (c - beta_1 * alpha_2) / beta_2;
//         const double a1_cand_2 = (b - beta_2 - beta_1) / alpha_2;
//         const double a1_cand_3 = a - alpha_2;

//         a1_cands = {a1_cand_3, a1_cand_1, a1_cand_2};
//         a2_cands = {alpha_2, alpha_2, alpha_2};
//       } else {
//         const double a2_cand_1 = (c - alpha_1 * beta_2) / beta_1;
//         const double a2_cand_2 = (b - beta_2 - beta_1) / alpha_1;
//         const double a2_cand_3 = a - alpha_1;

//         a1_cands = {alpha_1, alpha_1, alpha_1};
//         a2_cands = {a2_cand_3, a2_cand_1, a2_cand_2};
//       }

//       double eps_q_best = 0;

//       for (int i = 0; i < 3; i++) {
//         const double a1 = a1_cands[i];
//         const double a2 = a2_cands[i];

//         if (std::isfinite(a1) && std::isfinite(a2)) {
//           const double eps_q = calc_eps_q(a1, beta_1, a2, beta_2);

//           if (i == 0 || eps_q < eps_q_best) {
//             alpha_1 = a1;
//             alpha_2 = a2;
//             eps_q_best = eps_q;
//           }
//         }
//       }
//     }
//   } else if (d_2 == 0) {
//     const double d_3 = d - l_3 * l_3;

//     alpha_1 = l_1;
//     beta_1 = l_3 + std::sqrt(-d_3);
//     alpha_2 = l_1;
//     beta_2 = l_3 - std::sqrt(-d_3);

//     if (std::abs(beta_1) > std::abs(beta_2)) {
//       beta_2 = d / beta_1;
//     } else if (std::abs(beta_2) > std::abs(beta_1)) {
//       beta_1 = d / beta_2;
//     }
//   } else {
//     // No real solutions
//     return std::vector<double>{};
//   }

//   double eps_t = calc_eps_t(alpha_1, beta_1, alpha_2, beta_2);

//   for (int i = 0; i < 8; i++) {
//     if (eps_t == 0) {
//       break;
//     }

//     const double f_0 = beta_1 * beta_2 - d;
//     const double f_1 = beta_1 * alpha_2 + alpha_1 * beta_2 - c;
//     const double f_2 = beta_1 + alpha_1 * alpha_2 + beta_2 - b;
//     const double f_3 = alpha_1 + alpha_2 - a;
//     const double c_1 = alpha_1 - alpha_2;
//     const double det_j = beta_1 * beta_1 - beta_1 * (alpha_2 * c_1 + 2 * beta_2) + beta_2 * (alpha_1 * c_1 + beta_2);

//     if (det_j == 0) {
//       break;
//     }

//     const double inv = 1 / det_j;
//     const double c_2 = beta_2 - beta_1;
//     const double c_3 = beta_1 * alpha_2 - alpha_1 * beta_2;
//     const double dz_0 = c_1 * f_0 + c_2 * f_1 + c_3 * f_2 - (beta_1 * c_2 + alpha_1 * c_3) * f_3;
//     const double dz_1 = (alpha_1 * c_1 + c_2) * f_0 - beta_1 * (c_1 * f_1 + c_2 * f_2 + c_3 * f_3);
//     const double dz_2 = -c_1 * f_0 - c_2 * f_1 - c_3 * f_2 + (alpha_2 * c_3 + beta_2 * c_2) * f_3;
//     const double dz_3 = -(alpha_2 * c_1 + c_2) * f_0 + beta_2 * (c_1 * f_1 + c_2 * f_2 + c_3 * f_3);
//     const double a1 = alpha_1 - inv * dz_0;
//     const double b1 = beta_1 - inv * dz_1;
//     const double a2 = alpha_2 - inv * dz_2;
//     const double b2 = beta_2 - inv * dz_3;
//     const double new_eps_t = calc_eps_t(a1, b1, a2, b2);

//     if (new_eps_t < eps_t) {
//       alpha_1 = a1;
//       beta_1 = b1;
//       alpha_2 = a2;
//       beta_2 = b2;
//       eps_t = new_eps_t;
//     } else {
//       break;
//     }
//   }

//   return std::vector<double>{alpha_1, beta_1, alpha_2, beta_2};
// }

// /// Returns an array of candidate cubics matching given metrics.
// std::vector<dcubic_bezier> cubic_fit(const double th0, const double th1, const double area, const double mx) {
//   const double c0 = std::cos(th0);
//   const double s0 = std::sin(th0);
//   const double c1 = std::cos(th1);
//   const double s1 = std::sin(th1);

//   const double a4 = -9 * c0 * (((2 * s1 * c1 * c0 + s0 * (2 * c1 * c1 - 1)) * c0 - 2 * s1 * c1) * c0 - c1 * c1 * s0);
//   const double a3 = 12 *
//     ((((c1 * (30 * area * c1 - s1) - 15 * area) * c0 + 2 * s0 - c1 * s0 * (c1 + 30 * area * s1)) * c0 + c1 * (s1 - 15 * area * c1)
//      ) *
//        c0 -
//      s0 * c1 * c1);
//   const double a2 = 12 *
//     ((((70 * mx + 15 * area) * s1 * s1 + c1 * (9 * s1 - 70 * c1 * mx - 5 * c1 * area)) * c0 -
//       5 * s0 * s1 * (3 * s1 - 4 * c1 * (7 * mx + area))) *
//        c0 -
//      c1 * (9 * s1 - 70 * c1 * mx - 5 * c1 * area));
//   const double a1 = 16 *
//     (((12 * s0 - 5 * c0 * (42 * mx - 17 * area)) * s1 - 70 * c1 * (3 * mx - area) * s0 - 75 * c0 * c1 * area * area) * s1 -
//      75 * c1 * c1 * area * area * s0);
//   const double a0 = 80 * s1 * (42 * s1 * mx - 25 * area * (s1 - c1 * area));

//   std::vector<double> roots;

//   if (std::abs(a4) > EPS) {
//     const double a = a3 / a4;
//     const double b = a2 / a4;
//     const double c = a1 / a4;
//     const double d = a0 / a4;

//     const std::optional<std::vector<double>> quads = factor_quartic_inner(a, b, c, d, false);

//     /*
//     const solved = solve_quartic(a0, a1, a2, a3, a4);
//     for (let x of solved) {
//         const y = (((a4 * x + a3) * x + a2) * x + a1) * x + a0;
//         console.log(x, y);
//     }
//     */

//     roots = {};

//     if (quads.has_value()) {
//       for (int i = 0; i < quads->size(); i += 2) {
//         const double c1 = quads->at(i);
//         const double c0 = quads->at(i + 1);

//         const math::QuadraticSolutions<double> q_roots = math::solve_quadratic(1.0, c1, c0);

//         if (q_roots.count > 0) {
//           for (int j = 0; j < q_roots.count; j++) {
//             roots.push_back(q_roots[j]);
//           }
//         } else {
//           // Real part of pair of complex roots
//           roots.push_back(-0.5 * c1);
//         }
//       }
//     }
//   } else {
//     // Question: do we ever care about complex roots in these cases?
//     if (std::abs(a3) > EPS) {
//       const math::CubicSolutions<double> c_roots = math::solve_cubic(a3, a2, a1, a0);

//       for (int j = 0; j < c_roots.count; j++) {
//         roots.push_back(c_roots[j]);
//       }
//     } else {
//       const math::QuadraticSolutions<double> q_roots = math::solve_quadratic(a2, a1, a0);

//       for (int j = 0; j < q_roots.count; j++) {
//         roots.push_back(q_roots[j]);
//       }
//     }
//   }

//   const double s01 = s0 * c1 + s1 * c0;
//   std::vector<dcubic_bezier> cubics = {};

//   for (double d0 : roots) {
//     double d1 = (2 * d0 * s0 - area * (20 / 3.)) / (d0 * s01 - 2 * s1);

//     if (d0 < 0) {
//       d0 = 0;
//       d1 = s0 / s01;
//     } else if (d1 < 0) {
//       d0 = s1 / s01;
//       d1 = 0;
//     }

//     if (d0 >= 0 && d1 >= 0) {
//       cubics.push_back(dcubic_bezier{{0, 0}, {d0 * c0, d0 * s0}, {1 - d1 * c1, d1 * s1}, {1, 0}});
//     }
//   }

//   return cubics;
// }

// math::CubicSolutions<double> intersect_ray(const dcubic_bezier& c, const dvec2 p, const dvec2 d) {
//   const double px0 = c.p0.x;
//   const double px1 = 3 * c.p1.x - 3 * c.p0.x;
//   const double px2 = 3 * c.p2.x - 6 * c.p1.x + 3 * c.p0.x;
//   const double px3 = c.p3.x - 3 * c.p2.x + 3 * c.p1.x - c.p0.x;
//   const double py0 = c.p0.y;
//   const double py1 = 3 * c.p1.y - 3 * c.p0.y;
//   const double py2 = 3 * c.p2.y - 6 * c.p1.y + 3 * c.p0.y;
//   const double py3 = c.p3.y - 3 * c.p2.y + 3 * c.p1.y - c.p0.y;

//   const double c0 = d.y * (px0 - p.x) - d.x * (py0 - p.y);
//   const double c1 = d.y * px1 - d.x * py1;
//   const double c2 = d.y * px2 - d.x * py2;
//   const double c3 = d.y * px3 - d.x * py3;

//   return math::solve_cubic_normalized(c3, c2, c1, c0);
// }

// struct Cusp {
//   double t0;
//   double t1;
//   double sign;
// };

// struct CuspAccumulator {

//   dquadratic_bezier q;

//   double c0;
//   double c1;
//   double c2;

//   double d;

//   double t0;
//   double last_t;
//   double last_y;

//   std::vector<Cusp> result;

//   CuspAccumulator(const dquadratic_bezier& q, const double c0, const double c1, const double c2, const double d) :
//     q(q), c0(c0), c1(c1), c2(c2), d(d), t0(0.0), last_t(0.0), last_y(0.0) {
//     last_y = eval(0.0);
//   }

//   double eval(const double t) const {
//     const double ds2 = math::squared_length(q.sample(t));
//     const double k = (((c2 * t + c1) * t) + c0) / (ds2 * std::sqrt(ds2));
//     return k * d + 1.0;
//   }

//   void report(const double t) {
//     const double yb = eval(t);
//     const double ya = last_y;

//     if (ya >= 0.0 != yb >= 0.0) {
//       const double tx = (yb * last_t - ya * t) / (yb - ya);
//       result.push_back(Cusp{t0, tx, math::sign(ya)});
//       t0 = tx;
//     }

//     last_t = t;
//     last_y = yb;
//   }

//   std::vector<Cusp>& reap() {
//     result.push_back(Cusp{t0, 1.0, math::sign(last_y)});
//     return result;
//   }
// };

// template <typename T>
// struct CubicOffset {
//   CubicPath<T>& sink;

//   dcubic_bezier c;
//   dquadratic_bezier q;

//   double d;

//   CubicOffset(const dcubic_bezier& c, const double d, CubicPath<T>& sink) : c(c), q(lower(c)), d(d), sink(sink) { }

//   struct Approximation {
//     dcubic_bezier c;
//     double err;
//   };

//   struct Metrics {
//     double arclen;
//     double area;
//     double moment_x;
//   };

//   struct Sample {
//     double arclen;
//     dvec2 p;
//     dvec2 d;
//   };

//   struct RotatedCubicOffset {
//     CubicOffset c;
//     double th;
//     dvec2 p0;

//     RotatedCubicOffset(const CubicOffset c, const double th, const dvec2& p0) : c(c), th(th), p0(p0) { }
//   };

//   dvec2 eval_offset(const double t) const {
//     const dvec2 dp = q.sample(t);
//     const double s = d / math::length(dp);
//     return dvec2{-s * dp.y, s * dp.x};
//   }

//   dvec2 eval(const double t) const { return c.sample(t) + eval_offset(t); }

//   dvec2 eval_deriv(const double t) const {
//     const dvec2 dp = q.sample(t);
//     const dvec2 ddp = q.derivative(t);

//     const double h = math::squared_length(dp);
//     const double turn = math::cross(ddp, dp) * d / (h * std::sqrt(h));
//     const double s = 1.0 + turn;

//     return s * dp;
//   }

//   Metrics calc() const {
//     double arclen = 0.0;
//     double area = 0.0;
//     double moment_x = 0.0;

//     const double* co = GAUSS_LEGENDRE_COEFFS_32;

//     for (int i = 0; i < 32; i += 2) {
//       const double t = 0.5 * (1.0 + co[i + 1]);
//       const double wi = co[i];

//       const dvec2 dp = eval_deriv(t);
//       const dvec2 p = eval(t);

//       const double d_area = wi * dp.x * p.y;

//       arclen += wi * math::length(dp);
//       area += d_area;
//       moment_x += p.x * d_area;
//     }

//     return Metrics{0.5 * arclen, 0.5 * area, 0.5 * moment_x};
//   }

//   std::vector<Sample> sample_pts(const int n) const {
//     std::vector<Sample> result = {};
//     double arclen = 0.0;

//     // Probably overkill, but keep it simple
//     const double* co = GAUSS_LEGENDRE_COEFFS_32;
//     const double dt = 1.0 / (static_cast<double>(n) + 1.0);

//     for (int i = 0; i < n; i++) {
//       for (int j = 0; j < 32; j += 2) {
//         const double t = dt * (i + 0.5 + 0.5 * co[j + 1]);
//         arclen += co[j] * math::length(eval_deriv(t));
//       }

//       const double t = dt * (i + 1);
//       const dvec2 d = eval_offset(t);
//       const dvec2 p = c.sample(t) + d;

//       result.push_back(Sample{arclen * 0.5 * dt, p, d});
//     }

//     return result;
//   }

//   RotatedCubicOffset rotate_to_x() {
//     const dvec2 p0 = c.p0 + eval_offset(0.0);
//     const dvec2 p1 = c.p3 + eval_offset(1.0);

//     const double th = math::atan2(p0, p1);
//     const dmat2x3 a = dmat2x3::from_rotation(std::sin(-th), std::cos(-th));

//     return RotatedCubicOffset{
//       CubicOffset(dcubic_bezier{a * (c.p0 - p0), a * (c.p1 - p0), a * (c.p2 - p0), a * (c.p3 - p0)}, d, sink),
//       th,
//       p0
//     };
//   }

//   double est_cubic_err(const dcubic_bezier& cu, const std::vector<Sample>& samples, const double tolerance) {
//     double err = 0;
//     double tol2 = tolerance * tolerance;

//     for (auto& sample : samples) {
//       std::optional<double> best_err = std::nullopt;
//       // Project sample point onto approximate curve along normal.
//       math::CubicSolutions<double> intersections = intersect_ray(cu, sample.p, sample.d);

//       if (intersections.count == 0) {
//         // In production, if no rays intersect we probably want
//         // to reject this candidate altogether. But we sample the
//         // endpoints so you can get a plausible number.
//         intersections = math::CubicSolutions<double>{0, 1};
//       }

//       for (int i = 0; i < intersections.count; i++) {
//         const double t = intersections[i];
//         const dvec2 p_proj = cu.sample(t);
//         const double this_err = math::squared_length(sample.p - p_proj);

//         if (!best_err.has_value() || this_err < best_err) {
//           best_err = this_err;
//         }
//       }

//       err = std::max(err, best_err.value());

//       if (err > tol2) {
//         break;
//       }
//     }

//     return std::sqrt(err);
//   }

//   std::optional<Approximation> cubic_approx(const double tolerance, const double sign) {
//     const RotatedCubicOffset r = rotate_to_x();

//     const Metrics metrics = r.c.calc();

//     const double end_x = r.c.c.p3.x + r.c.eval_offset(1.0).x;
//     const double arclen = metrics.arclen;

//     const double th0 = std::atan2(sign * r.c.q.p0.y, sign * r.c.q.p0.x);
//     const double th1 = -std::atan2(sign * r.c.q.p2.y, sign * r.c.q.p2.x);
//     const double ex2 = end_x * end_x;
//     const double ex3 = ex2 * end_x;

//     const std::vector<dcubic_bezier> cands = cubic_fit(th0, th1, metrics.area / ex2, metrics.moment_x / ex3);
//     const double cx = end_x * std::cos(r.th);
//     const double sx = end_x * std::sin(r.th);

//     const dmat2x3 a = {cx, -sx, r.p0.x, sx, cx, r.p0.y};

//     const std::vector<Sample> samples = sample_pts(10);

//     std::optional<dcubic_bezier> best_c = std::nullopt;
//     std::vector<double> errs = {};

//     double best_err;

//     for (auto& raw_cand : cands) {
//       const dcubic_bezier cand = {a * raw_cand.p0, a * raw_cand.p1, a * raw_cand.p2, a * raw_cand.p3};
//       const double err = est_cubic_err(cand, samples, tolerance);

//       errs.push_back(err);

//       if (!best_c.has_value() || err < best_err) {
//         best_err = err;
//         best_c = cand;
//       }
//     }

//     if (!best_c.has_value()) {
//       return std::nullopt;
//     }

//     return Approximation{best_c.value(), best_err};
//   }

//   void cubic_approx_seq(const double tolerance, const double sign) {
//     const std::optional<Approximation> approx = cubic_approx(tolerance, sign);

//     if (approx.has_value() && approx->err <= tolerance) {
//       sink.cubic_to(math::Vec2<T>(approx->c.p1), math::Vec2<T>(approx->c.p2), math::Vec2<T>(approx->c.p3));
//     } else {
//       const auto [left, right] = split(c, 0.5);

//       CubicOffset(left, d, sink).cubic_approx_seq(tolerance, sign);
//       CubicOffset(right, d, sink).cubic_approx_seq(tolerance, sign);
//     }
//   }
// };

// static inline dquadratic_bezier lower(const dcubic_bezier& c) {
//   return {3 * (c.p1 - c.p0), 3 * (c.p2 - c.p1), 3 * (c.p3 - c.p2)};
// }

// static inline double tri_sign(const dvec2 a, const dvec2 b) { return b.x * (a.y - b.y) - b.y * (a.x - b.x); }

// static inline double line_nearest_origin(const dvec2 a, const dvec2 b) {
//   const dvec2 d = b - a;

//   double dotp = -math::dot(a, d);
//   double d_sq = math::squared_length(d);

//   if (dotp <= 0.0) {
//     return math::squared_length(a);
//   } else if (dotp >= d_sq) {
//     return math::squared_length(b);
//   } else {
//     const double t = dotp / d_sq;
//     const dvec2 p = a + t * d;

//     return math::squared_length(p);
//   }
// }

// static void find_offset_cusps_rec(
//   const dcubic_bezier& c,
//   CuspAccumulator& cusps,
//   const double t0,
//   const double t1,
//   const double c0,
//   const double c1,
//   const double c2,
//   const double d
// ) {
//   cusps.report(t0);

//   const dquadratic_bezier q = lower(extract(c, t0, t1));

//   // compute interval for ds/dt, using convex hull of hodograph
//   const double dt = t1 - t0;
//   const double d1 = tri_sign(q.p0, q.p1);
//   const double d2 = tri_sign(q.p1, q.p2);
//   const double d3 = tri_sign(q.p2, q.p0);

//   const bool z = !((d1 < 0 || d2 < 0 || d3 < 0) && (d1 > 0 || d2 > 0 || d3 > 0));

//   const double ds0 = math::squared_length(q.p0);
//   const double ds1 = math::squared_length(q.p1);
//   const double ds2 = math::squared_length(q.p2);
//   const double max_ds = std::sqrt(std::max({ds0, ds1, ds2})) / dt;

//   const double m1 = line_nearest_origin(q.p0, q.p1);
//   const double m2 = line_nearest_origin(q.p1, q.p2);
//   const double m3 = line_nearest_origin(q.p2, q.p0);
//   const double min_ds = z ? 0 : std::sqrt(std::min({m1, m2, m3})) / dt;

//   double cmin = std::min(c0, c0 + c1 + c2);
//   double cmax = std::max(c0, c0 + c1 + c2);

//   const double t_crit = -0.5 * c1 / c2;

//   if (t_crit > 0 && t_crit < 1) {
//     const double c_at_t = (c2 * t_crit + c1) * t_crit + c0;
//     cmin = std::min(cmin, c_at_t);
//     cmax = std::max(cmax, c_at_t);
//   }

//   const double min3 = min_ds * min_ds * min_ds;
//   const double max3 = max_ds * max_ds * max_ds;

//   // TODO: signs are wrong, want min/max of c * d
//   // But this is a suitable starting place for clipping.
//   if (cmin * d > -min3 || cmax * d < -max3) {
//     // return;
//   }

//   const math::QuadraticSolutions<double> rmax = math::solve_quadratic(c2 * d, c1 * d, c0 * d + max3);
//   const math::QuadraticSolutions<double> rmin = math::solve_quadratic(c2 * d, c1 * d, c0 * d + min3);

//   std::vector<double> ts;

//   // TODO: count = 1 cases. Also maybe reduce case explosion?
//   if (rmax.count == 2 && rmin.count == 2) {
//     if (c2 > 0) {
//       ts = {rmin[0], rmax[0], rmax[1], rmin[1]};
//     } else {
//       ts = {rmax[0], rmin[0], rmin[1], rmax[1]};
//     }
//   } else if (rmin.count == 2) {
//     if (c2 > 0) {
//       ts = {rmin[0], rmin[1]};
//     } else {
//       ts = {t0, rmin[0], rmin[1], t1};
//     }
//   } else if (rmax.count == 2) {
//     if (c2 > 0) {
//       ts = {t0, rmax[0], rmax[1], t1};
//     } else {
//       ts = {rmax[0], rmax[1]};
//     }
//   } else {
//     const double c_at_t0 = (c2 * t0 + c1) * t0 + c0;
//     if (c_at_t0 * d < -min3 && c_at_t0 * d > -max3) {
//       ts = {t0, t1};
//     } else {
//       ts = {};
//     }
//   }

//   for (int i = 0; i < ts.size(); i += 2) {
//     const double new_t0 = std::max(t0, ts[i]);
//     const double new_t1 = std::min(t1, ts[i + 1]);

//     if (new_t1 > new_t0) {
//       if (new_t1 - new_t0 < 1e-9) {
//         cusps.report(new_t0);
//         cusps.report(new_t1);
//       } else if (new_t1 - new_t0 > 0.5 * dt) {
//         const double tm = 0.5 * (new_t0 + new_t1);
//         find_offset_cusps_rec(c, cusps, new_t0, tm, c0, c1, c2, d);
//         find_offset_cusps_rec(c, cusps, tm, new_t1, c0, c1, c2, d);
//       } else {
//         find_offset_cusps_rec(c, cusps, new_t0, new_t1, c0, c1, c2, d);
//       }
//     }
//   }

//   cusps.report(t1);
// }

// static std::vector<Cusp> find_offset_cusps(const dcubic_bezier& c, const double d) {
//   const dquadratic_bezier q = lower(c);

//   const dvec2 d0 = q.p0;
//   const dvec2 d1 = 2.0 * (q.p1 - q.p0);
//   const dvec2 d2 = q.p0 - 2.0 * q.p1 + q.p2;

//   const double c0 = math::cross(d1, d0);
//   const double c1 = math::cross(d2, d0) * 2.0;
//   const double c2 = math::cross(d2, d1);

//   CuspAccumulator cusps(q, c0, c1, c2, d);

//   find_offset_cusps_rec(c, cusps, 0.0, 1.0, c0, c1, c2, d);

//   return cusps.reap();
// }

// template <typename T, typename _>
// void offset_cubic(const dcubic_bezier& curve, const double offset, const double tolerance, CubicPath<T>& sink) {
//   const std::vector<Cusp> cusps = find_offset_cusps(curve, offset);

//   for (const auto& cusp : cusps) {
//     CubicOffset(extract(curve, cusp.t0, cusp.t1), offset, sink).cubic_approx_seq(tolerance, cusp.sign);
//   }

//   for (const auto& cusp : cusps) {
//     const dvec2 p0 = curve.sample(cusp.t0);
//     const dvec2 p1 = curve.sample(cusp.t1);

//     renderer::Renderer::draw_rect(
//       vec2(p0),
//       vec2(4.0f),
//       offset >= 0.0 ? vec4(0.2f, 0.8f, 0.2f, 1.0f) : vec4(0.8f, 0.2f, 0.2f, 1.0f)
//     );
//     renderer::Renderer::draw_rect(
//       vec2(p1),
//       vec2(4.0f),
//       offset >= 0.0 ? vec4(0.2f, 0.8f, 0.2f, 1.0f) : vec4(0.8f, 0.2f, 0.2f, 1.0f)
//     );
//   }
// }

// /* -- Template Instantiation -- */

// template void offset_cubic(const dcubic_bezier&, const double, const double, CubicPath<float>&);
// template void offset_cubic(const dcubic_bezier&, const double, const double, CubicPath<double>&);
// }
