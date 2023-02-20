#pragma once

#include "geometry.h"
#include "bezier_fitter.h"

inline extern float max_angle = MATH_PI / 100.0f;
inline extern float stroke_width = 2.0f;

Geometry stroke_freehand_path(const std::vector<FreehandPathPoint>& points, float thickness, float zoom);

std::vector<FreehandPathPoint> smooth_freehand_path(const std::vector<FreehandPathPoint>& points, int kernel_size);

void stroke_curve(const Bezier& curve, uint32_t& offset, Geometry& geo);

Geometry stroke_curves(const std::vector<Bezier>& curves);
