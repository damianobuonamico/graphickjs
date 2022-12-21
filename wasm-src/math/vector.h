#pragma once

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

vec2 min(const vec2& v1, const vec2& v2);
vec3 min(const vec3& v1, const vec3& v2);
vec4 min(const vec4& v1, const vec4& v2);

vec2 max(const vec2& v1, const vec2& v2);
vec3 max(const vec3& v1, const vec3& v2);
vec4 max(const vec4& v1, const vec4& v2);

float length(const vec2& v);
float length(const vec3& v);
float length(const vec4& v);

float distance(const vec2& v1, const vec2& v2);
float distance(const vec3& v1, const vec3& v2);
float distance(const vec4& v1, const vec4& v2);

vec2 lerp(const vec2& v1, const vec2& v2, float t);
vec3 lerp(const vec3& v1, const vec3& v2, float t);
vec4 lerp(const vec4& v1, const vec4& v2, float t);

#include "vector.inl"
