#pragma once

#include "mat2.h"
#include "mat3.h"
#include "mat4.h"

float determinant(const mat2& m);
float determinant(const mat3& m);
float determinant(const mat4& m);

mat2 inverse(const mat2& m);
mat3 inverse(const mat3& m);
mat4 inverse(const mat4& m);

// #define det determinant
// #define inv inverse

#include "matrix.inl"
