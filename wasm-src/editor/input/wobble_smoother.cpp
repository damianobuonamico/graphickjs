#include "wobble_smoother.h"

#include "../../math/vector.h"
#include "../../math/math.h"
#include "../../utils/console.h"

std::deque<WobbleSmoother::Sample> WobbleSmoother::s_samples{};
vec2 WobbleSmoother::s_weighted_position_sum = { 0.0f, 0.0f };
float WobbleSmoother::s_distance_sum = 0.0f;
float WobbleSmoother::s_weighted_pressure_sum = 0.0f;
double WobbleSmoother::s_duration_sum = 0.0;
WobbleSmoother::WobbleSmootherParams WobbleSmoother::s_params{};

void WobbleSmoother::reset(const WobbleSmootherParams& params, const vec2& position, float pressure, double time) {
  s_params = params;

  s_samples.clear();
  s_weighted_position_sum = { 0.0f, 0.0f };
  s_weighted_pressure_sum = 0.0f;
  s_distance_sum = 0.0f;
  s_duration_sum = 0.0;

  s_samples.push_back({ position, { 0.0f, 0.0f }, pressure, 0.0f, 0.0f, 0.0f, time });
}

void WobbleSmoother::reset(const vec2& position, float pressure, double time) {
  reset({}, position, pressure, time);
}

vec3 WobbleSmoother::update(const vec2& position, float pressure, double time) {
  double delta_time = time - s_samples.back().time;
  float dist = distance(position, s_samples.back().position);
  float prev_pressure = s_samples.back().pressure;

  Sample sample = {
    position,
    position * (float)delta_time,
    pressure,
    pressure * (float)delta_time,
    dist,
    delta_time,
    time
  };


  s_samples.push_back(sample);

  s_weighted_position_sum += sample.weighted_position;
  s_weighted_pressure_sum += sample.weighted_pressure;
  s_distance_sum += sample.distance;
  s_duration_sum += sample.duration;

  while (s_samples.front().time < time - s_params.timeout) {
    s_weighted_position_sum -= s_samples.front().weighted_position;
    s_weighted_pressure_sum -= s_samples.front().weighted_pressure;
    s_distance_sum -= s_samples.front().distance;
    s_duration_sum -= s_samples.front().duration;

    s_samples.pop_front();
  }

  if (s_duration_sum == 0.0f) {
    return { position.x, position.y, pressure };
  }

  vec2 average_position = s_weighted_position_sum / (float)s_duration_sum;
  float average_speed = s_distance_sum / (float)s_duration_sum;
  float average_pressure = s_weighted_pressure_sum / (float)s_duration_sum;

  float t = normalize(average_speed);
  vec2 value = lerp(average_position, position, t);

  if (s_params.simulate_pressure) {
    float sp = std::min(1.0f, dist / s_params.width);
    float rp = std::min(1.0f, 1.0f - sp);
    return { value.x, value.y, std::min(1.0f, prev_pressure + (rp - prev_pressure) * (sp * 0.675f)) };
  }

  return { value.x, value.y, lerp(average_pressure, pressure, t) };
}

float WobbleSmoother::normalize(float value) {
  if (s_params.speed_floor == s_params.speed_ceiling) {
    return value > s_params.speed_floor ? 1.0f : 0.0f;
  }

  return clamp((value - s_params.speed_floor) / (s_params.speed_ceiling - s_params.speed_floor), 0.0f, 1.0f);
}
