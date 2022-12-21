#include "wobble_smoother.h"

#include "../../math/vector.h"
#include "../../math/math.h"

std::deque<WobbleSmoother::Sample> WobbleSmoother::s_samples{};
vec2 WobbleSmoother::s_weighted_position_sum = { 0.0f, 0.0f };
float WobbleSmoother::s_distance_sum = 0.0f;
float WobbleSmoother::s_duration_sum = 0.0f;
WobbleSmoother::WobbleSmootherParams WobbleSmoother::s_params{};

void WobbleSmoother::reset(const WobbleSmootherParams& params, const vec2& position, float time) {
  s_params = params;

  s_samples.clear();
  s_weighted_position_sum = { 0.0f, 0.0f };
  s_distance_sum = 0.0f;
  s_duration_sum = 0.0f;

  s_samples.push_back({ position, {0.0f, 0.0f}, 0.0f, 0.0f, time });
}

void WobbleSmoother::reset(const vec2& position, float time) {
  reset({}, position, time);
}

vec2 WobbleSmoother::update(const vec2& position, float time) {
  float delta_time = time - s_samples.back().time;

  s_samples.push_back({ position, position * delta_time, distance(position, s_samples.back().position), delta_time, time });

  s_weighted_position_sum += s_samples.back().weighted_position;
  s_distance_sum += s_samples.back().distance;
  s_duration_sum += s_samples.back().duration;

  while (s_samples.front().time < time - s_params.timeout) {
    s_weighted_position_sum -= s_samples.front().weighted_position;
    s_distance_sum -= s_samples.front().distance;
    s_duration_sum -= s_samples.front().duration;

    s_samples.pop_front();
  }

  if (s_duration_sum == 0.0f) {
    return position;
  }

  vec2 average_position = s_weighted_position_sum / s_duration_sum;
  float average_speed = s_distance_sum / s_duration_sum;

  return lerp(average_position, position, normalize(average_speed));
}

float WobbleSmoother::normalize(float value) {
  if (s_params.speed_floor == s_params.speed_ceiling) {
    return value > s_params.speed_floor ? 1.0f : 0.0f;
  }

  return clamp((value - s_params.speed_floor) / (s_params.speed_ceiling - s_params.speed_floor), 0.0f, 1.0f);
}
