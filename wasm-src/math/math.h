#pragma once

#include <stdint.h>

#define MATH_PI 3.14159265358979323846

float round(float t, float decimals) noexcept;
float map(float t, float old_min, float old_max, float new_min, float new_max);
float clamp(float t, float min, float max);
float sign(float t);

#include "math.inl"
