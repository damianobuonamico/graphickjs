#pragma once

#include <stdint.h>

float round(float t, float decimals) noexcept;
float map(float t, float old_min, float old_max, float new_min, float new_max);
float clamp(float t, float min, float max);

#include "math.inl"
