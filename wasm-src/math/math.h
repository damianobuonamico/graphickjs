#pragma once

#include <stdint.h>

float round(float t, uint8_t decimals) noexcept;
float map(float t, float old_min, float old_max, float new_min, float new_max);

#include "math.inl"
