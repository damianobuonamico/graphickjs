#pragma once

#include <stdint.h>
#include <float.h>
#include <cmath>

#define MATH_PI 3.14159265358979323846f
#define MATH_TWO_PI 3.14159265358979323846f * 2.0f

float round(float t, float decimals) noexcept;
float map(float t, float old_min, float old_max, float new_min, float new_max);
float clamp(float t, float min, float max);
float sign(float t);
float lerp(float a, float b, float t);

bool is_almost_zero(const double t, const float eps);
bool is_almost_equal(const float t1, const float t2, const float eps);
bool is_normalized(const float t, bool include_ends);
bool is_in_range(const float t, const float min, const float max, bool include_ends);

#include "scalar.inl"
