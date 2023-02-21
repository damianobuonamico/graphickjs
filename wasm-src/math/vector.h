#pragma once

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

vec2 min(const vec2& v1, const vec2& v2);
vec3 min(const vec3& v1, const vec3& v2);
vec4 min(const vec4& v1, const vec4& v2);
void min(const vec2& v1, const vec2& v2, vec2& out);
void min(const vec3& v1, const vec3& v2, vec3& out);
void min(const vec4& v1, const vec4& v2, vec4& out);

vec2 max(const vec2& v1, const vec2& v2);
vec3 max(const vec3& v1, const vec3& v2);
vec4 max(const vec4& v1, const vec4& v2);
void max(const vec2& v1, const vec2& v2, vec2& out);
void max(const vec3& v1, const vec3& v2, vec3& out);
void max(const vec4& v1, const vec4& v2, vec4& out);

float length(const vec2& v);
float length(const vec3& v);
float length(const vec4& v);

float squared_length(const vec2& v);
float squared_length(const vec3& v);
float squared_length(const vec4& v);

float distance(const vec2& v1, const vec2& v2);
float distance(const vec3& v1, const vec3& v2);
float distance(const vec4& v1, const vec4& v2);

float squared_distance(const vec2& v1, const vec2& v2);
float squared_distance(const vec3& v1, const vec3& v2);
float squared_distance(const vec4& v1, const vec4& v2);

vec2 lerp(const vec2& v1, const vec2& v2, float t);
vec3 lerp(const vec3& v1, const vec3& v2, float t);
vec4 lerp(const vec4& v1, const vec4& v2, float t);

vec2 bezier(const vec2& v1, const vec2& v2, const vec2& v3, const vec2& v4, float t);
vec2 bezier_derivative(const vec2& v1, const vec2& v2, const vec2& v3, const vec2& v4, float t);
vec2 bezier_second_derivative(const vec2& v1, const vec2& v2, const vec2& v3, const vec2& v4, float t);

vec2 midpoint(const vec2& v1, const vec2& v2);
vec3 midpoint(const vec3& v1, const vec3& v2);
vec4 midpoint(const vec4& v1, const vec4& v2);

vec2 normalize(const vec2& v);
vec3 normalize(const vec3& v);
vec4 normalize(const vec4& v);
vec2& normalize(const vec2& v, vec2& out);
vec3& normalize(const vec3& v, vec3& out);
vec4& normalize(const vec4& v, vec4& out);

vec2 normalize_length(const vec2& v, float t);
vec3 normalize_length(const vec3& v, float t);
vec4 normalize_length(const vec4& v, float t);
vec2& normalize_length(const vec2& v, float t, vec2& out);
vec3& normalize_length(const vec3& v, float t, vec3& out);
vec4& normalize_length(const vec4& v, float t, vec4& out);

vec2 negate(const vec2& v);
vec3 negate(const vec3& v);
vec4 negate(const vec4& v);
vec2& negate(const vec2& v, vec2& out);
vec3& negate(const vec3& v, vec3& out);
vec4& negate(const vec4& v, vec4& out);

vec2 abs(const vec2& v);
vec3 abs(const vec3& v);
vec4 abs(const vec4& v);
vec2& abs(const vec2& v, vec2& out);
vec3& abs(const vec3& v, vec3& out);
vec4& abs(const vec4& v, vec4& out);

void zero(const vec2& v);
void zero(const vec3& v);
void zero(const vec4& v);

bool is_zero(const vec2& v);
bool is_zero(const vec3& v);
bool is_zero(const vec4& v);

bool not_zero(const vec2& v);
bool not_zero(const vec3& v);
bool not_zero(const vec4& v);

bool is_almost_zero(const vec2& v, const float eps);
bool is_almost_zero(const vec3& v, const float eps);
bool is_almost_zero(const vec4& v, const float eps);

bool is_almost_equal(const vec2& v1, const vec2& v2, const float eps);
bool is_almost_equal(const vec3& v1, const vec3& v2, const float eps);
bool is_almost_equal(const vec4& v1, const vec4& v2, const float eps);

float dot(const vec2& v1, const vec2& v2);
float dot(const vec3& v1, const vec3& v2);
float dot(const vec4& v1, const vec4& v2);

float angle(const vec2& v1, const vec2& v2);
float atan2(const vec2& v1, const vec2& v2);

vec2 rotate(const vec2& v, const vec2& c, float t);

vec2 orthogonal(const vec2& v);
void orthogonal(const vec2& v, vec2& out);

vec2& swap_coordinates(const vec2& v, vec2& out);

// #define len length
// #define sq_len squared_length
// #define dist distance
// #define sq_dist squared_distance
// #define mid midpoint
// #define norm normalize
// #define norm_len normalize_length
// #define neg negate

#include "vector.inl"
