#pragma once

struct Settings {
  inline static float corners_radius_min = 0.5f;
  inline static float corners_radius_max = 2.45f;
  inline static float corners_angle_threshold = 3.14159265358979323846f / 8.0f;
  inline static float corners_min_distance = 5.0f;
  inline static unsigned int corners_samples_max = 5;

  inline static float simplification_threshold = 0.08f;
  inline static bool upsample_before_fitting = true;

  // inline static float max_fit_error = 0.216f;
  inline static float max_fit_error = 0.392f;

  inline static float mass_constant = 0.16f;
  inline static float spring_constant = 1.8f;
  inline static float viscosity_constant = 1.0f;

  inline static float facet_angle = 3.14159265358979323846f / 100.0f;
  inline static float tessellation_error = 0.2f;
};
