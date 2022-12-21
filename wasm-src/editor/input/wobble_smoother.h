#pragma once

#include "../../math/vec2.h"

#include <deque>

class WobbleSmoother {
public:
  struct WobbleSmootherParams {
    float timeout = 40.0f;
    float speed_floor = 1.31f;
    float speed_ceiling = 1.44f;
  };
public:
  WobbleSmoother() = delete;
  WobbleSmoother(const WobbleSmoother&) = delete;
  WobbleSmoother(WobbleSmoother&&) = delete;

  ~WobbleSmoother() = default;

  static void reset(const WobbleSmootherParams& params, const vec2& position, float time);
  static void reset(const vec2& position, float time);
  static vec2 update(const vec2& position, float time);
private:
  static float normalize(float value);
private:
  struct Sample {
    vec2 position = { 0.0f, 0.0f };
    vec2 weighted_position = { 0.0f, 0.0f };
    float distance = 0.0f;
    float duration = 0.0f;
    float time = 0.0f;
  };

  static std::deque<Sample> s_samples;
  static vec2 s_weighted_position_sum;
  static float s_distance_sum;
  static float s_duration_sum;

  static WobbleSmootherParams s_params;
};
