/**
 * @file defines.h
 * @brief The file contains the definition of the global constants.
 *
 * @todo refactor the constants
 */

#pragma once

#define GK_EPSILON 1e-6f
#define GK_POINT_EPSILON 1e-3f
#define GK_PATH_TOLERANCE 0.25
#define GK_NEWTON_ITERATIONS 5
#define GK_FIT_RESOLUTION 10
#define GK_MAX_RECURSION 16

#define GK_BUFFER_SIZE 131072
#define GK_LARGE_BUFFER_SIZE 524288

#define ZOOM_MIN 0.02f
#define ZOOM_MAX 640.0f
#define ZOOM_STEP 0.18f
#define PAN_STEP 36.0f

#define INPUT_MOVEMENT_THRESHOLD 4.0f
constexpr float INPUT_MOVEMENT_THRESHOLD_MULTIPLIER[3] = { 1.0f, 2.0f, 2.0f };

#define GEOMETRY_SQR_EPSILON 0.1f
#define GEOMETRY_CURVE_ERROR 0.2f
#define GEOMETRY_MAX_INTERSECTION_ERROR 1e-4f
#define GEOMETRY_MIN_FACET_ANGLE 0.01047197543f
#define GEOMETRY_BUTT_CAP_LENGTH 0.01f
#define GEOMETRY_CIRCLE_RATIO 0.55228474983079339840f

#define TOOL_CURVE_MOLDING_INTERPOLATION 0.44f
