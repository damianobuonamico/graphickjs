#include "stroker.h"

#include "../../math/vector.h"
#include "../../utils/defines.h"
#include "../renderer.h"

// TODO: fix graduated joins
// TODO: fix angle calculation when joining two vectors with the same direction
void tessellate_join(
  const TessellationParams& params,
  const vec2& point, const vec2& direction, const vec2& normal,
  float width, const uint32_t* override_end_index, Geometry& geo
) {
  uint32_t offset = geo.offset();
  float bend_direction = dot(direction, params.start_join_params.normal);

  vec2 h = 0.5f * (normal + params.start_join_params.normal);
  float height = length(h);
  float k = 2.0f * width - height;
  normalize(h, h);

  if (params.join == JoinType::Round) {
    float angle = std::acosf(dot(normal, params.start_join_params.normal) / (width * width));
    int increments = (int)std::ceilf(angle / params.rendering_options.facet_angle);

    if (increments > 1) {
      uint32_t end_index = override_end_index ? *override_end_index : offset + increments - 1;
      float increment = angle / (float)increments;
      vec2 bended_normal = params.start_join_params.normal;

      geo.push_vertex({ point, params.color, 0.0f, width });

      if (bend_direction < 0.0f) {
        geo.push_indices({ offset, params.start_join_params.index + 1, offset + 1 });

        increment = -increment;
        end_index++;
      } else {
        geo.push_indices({ offset, params.start_join_params.index, offset + 1 });

        negate(bended_normal, bended_normal);
        width = -width;
      }

      for (int i = 1; i < increments; ++i) {
        float angle_offset = (float)i * increment;
        float sin = std::sinf(angle_offset);
        float cos = std::cosf(angle_offset);
        vec2 p = {
          bended_normal.x * cos - bended_normal.y * sin,
          bended_normal.x * sin + bended_normal.y * cos
        };

        geo.push_vertex({ point + p, params.color, width });
        geo.push_indices({ offset, offset + i, offset + i - 1 });
      }

      geo.push_indices({ offset, end_index + 1, offset + increments - 1 });

      return;
    }
  }

  uint32_t end_index = override_end_index ? *override_end_index : offset;

  if (params.join == JoinType::Miter) {
    float cos = dot(normal, params.start_join_params.normal) / (width * width);
    float miter_length = width / std::sqrtf(0.5f * (1.0f + cos));

    if (miter_length < params.miter_limit * width) {
      vec2 miter = h * miter_length;

      if (bend_direction < 0.0f) {
        geo.push_vertex({ point + miter, params.color, 0.5f * (miter_length + width) });
        geo.push_indices({ params.start_join_params.index, params.start_join_params.index + 1, offset });
        geo.push_indices({ offset, end_index + 1, end_index + 2 });
      } else {
        geo.push_vertex({ point - miter, params.color, -0.5f * (miter_length + width) });
        geo.push_indices({ params.start_join_params.index, params.start_join_params.index + 1, offset });
        geo.push_indices({ offset, end_index + 1, end_index + 2 });
      }

      return;
    }
  }

  vec2 inset = h * k;

  if (bend_direction < 0.0f) {
    geo.push_vertex({ point - inset, params.color, -width });
    geo.push_indices({ params.start_join_params.index + 1, offset, end_index + 2 });
  } else {
    geo.push_vertex({ point + inset, params.color, width });
    geo.push_indices({ params.start_join_params.index, offset, end_index + 1 });
  }
}

// TODO: fix graduated caps
void tessellate_cap(
  const TessellationParams& params,
  const vec2& point, const vec2& normal,
  bool is_end_cap, float width, Geometry& geo
) {
  uint32_t offset = geo.offset();
  uint32_t end_index = params.start_join_params.index;

  if (params.cap == CapType::Round) {
    float angle = MATH_PI;
    int increments = (int)std::ceilf(angle / params.rendering_options.facet_angle);

    if (increments > 1) {
      float increment = angle / (float)increments;

      if (is_end_cap) {
        increment = -increment;
      } else {
        end_index = offset + increments + 1;
      }

      geo.reserve(increments + 1, (increments + 1) * 3);
      geo.push_vertex({ point, params.color, 0.0f, width });
      geo.push_indices({ offset, end_index + 1, offset + 1 });

      for (int i = 1; i <= increments; ++i) {
        float angle_offset = (float)i * increment;
        float sin = std::sinf(angle_offset);
        float cos = std::cosf(angle_offset);
        vec2 p = {
          normal.x * cos - normal.y * sin,
          normal.x * sin + normal.y * cos
        };

        geo.push_vertex({ point + p, params.color, width });
        geo.push_indices({ offset, offset + i, offset + i - 1 });
      }

      return;
    }
  }

  if (params.cap == CapType::Butt) {
    float cap_length = 120.0f * GEOMETRY_BUTT_CAP_LENGTH / params.rendering_options.zoom;
    vec2 normal_ortho = cap_length * orthogonal(normal) / (width);

    if (!is_end_cap) {
      end_index = offset + 4;
    } else {
      negate(normal_ortho, normal_ortho);
    }

    vec2 offset_normal = normal / width * (width - cap_length);

    vec2 A = point + offset_normal;
    vec2 B = point - offset_normal;

    geo.reserve(4, 6);
    geo.push_vertex({ A, params.color, 0.0f, cap_length });
    geo.push_vertex({ B, params.color, 0.0f, cap_length });
    geo.push_vertex({ B + normal_ortho, params.color, cap_length });
    geo.push_vertex({ A + normal_ortho, params.color, cap_length });
    geo.push_indices({ offset, offset + 1, offset + 2 });
    geo.push_indices({ offset, offset + 2, offset + 3 });

    return;
  }

  vec2 normal_ortho = orthogonal(normal);

  if (!is_end_cap) {
    end_index = offset + 4;
  } else {
    negate(normal_ortho, normal_ortho);
  }

  geo.reserve(4, 9);
  geo.push_vertex({ point, params.color, 0.0f, width });
  geo.push_vertex({ point + normal + normal_ortho, params.color, width });
  geo.push_vertex({ point - normal + normal_ortho, params.color, width });
  geo.push_vertex({ point - normal, params.color, width });
  geo.push_indices({ offset, offset + 1, offset + 2 });
  geo.push_indices({ offset, end_index + 1, offset + 1 });
  geo.push_indices({ offset, offset + 2, offset + 3 });
}
