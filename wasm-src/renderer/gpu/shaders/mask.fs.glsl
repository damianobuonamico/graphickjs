R"(

  precision highp float;

  uniform int uTileSize;

  in vec2 vFrom;
  in vec2 vTo;
  in vec2 vPos;

  out float oFragColor;

  float line(vec2 P, vec2 A, vec2 B, float r) {
    vec2 g = B - A;
    float d = dot(normalize(vec2(g.y, -g.x)), P - A);
    return smoothstep(0.0, 0.1, d);
  }

  void main() {
    // Determine winding, and sort into a consistent order so we only need to find one root below.
    // vec2 left = vFrom.x < vTo.x ? vFrom : vTo, right = vFrom.x < vTo.x ? vTo : vFrom;

    // Shoot a vertical ray toward the curve.
    // vec2 window = clamp(vec2(vFrom.x, vTo.x), -0.5, 0.5);
    // float offset = mix(window.x, window.y, 0.5) - left.x;
    // float t = offset / (right.x - left.x);

    // Compute position and derivative to form a line approximation.
    // float x = mix(left.x, right.x, t);
    // float y = mix(left.y, right.y, t);
    // float d = (right.y - left.y) / (right.x - left.x);

    // Look up area under that line, and scale horizontally to the window size.
    // float dX = window.x - window.y;
    // return texture(areaLUT, vec2(y + 8.0, abs(d * dX)) / 16.0) * dX;
    
    // oFragColor = vec4(vFrom.x, vFrom.y, 0.7, 1.0);

    float m = (vTo.y - vFrom.y) / (vTo.x - vFrom.x);
    float q = vFrom.y - m * vFrom.x;

    // float area = 1.0 - (vFrom.x + vTo.x) * (vTo.y - vFrom.y) * 0.5;

    float y = m * vPos.x + q;
    // float x = (vPos.y - q) / m;
    // if (x > vPos.x) {
    //   oFragColor = vec4(0.0, 0.0, 0.0, 0.0);
    // } else {
    //   oFragColor = vec4(vPos.x, vPos.y, 0.0, 1.0);
    // }

    // float fx = fwidth(vPos.x);

    // float y2 = m * (vPos.x + fx) + q;
    float w = y - vPos.y;

    float winding_sign = vFrom.x < vTo.x ? 1.0 : -1.0;
    float winding = vFrom.y < vTo.y ? -winding_sign : winding_sign; 
    vec2 from = vFrom.y < vTo.y ? vFrom : vTo;
    vec2 to = vFrom.y < vTo.y ? vTo : vFrom;
    // float winding = 1.0;

    // float factor = abs(m) > 999.0 ? 1.0 : smoothstep(0.0, 0.02, 10.0 * w / m * fwidth(w) * abs(vTo.y - vFrom.y));

    // oFragColor = vec4(1.0, 1.0, 1.0, 1.0) * smoothstep(0.945, 1.0, 1.0 - (vPos.x - x));
    
    float factor = abs(m) > 999.0 ? 1.0 : step(y, vPos.y);

    // if (abs(w) > 0.0) {
    //   oFragColor = 0.5;
    // } else {
    //   oFragColor = 1.0;
    // }
    float intensity = abs(m) > 999.0 ? 1.0 : line(vPos, from, to, 0.1);
    // oFragColor = intensity * winding;

    // oFragColor = vec4(area, 1.0, 1.0, 1.0);
    // if (area > 0.0) {
      // oFragColor = vec4(1.0, 1.0, 1.0, 1.0);
    // } else {
      // oFragColor = vec4(1.0, 0.0, 0.0, 1.0);
    // }

    // vec2 p1 = vPos;
    // vec2 p2 = p1 + 1.0 / float(uTileSize);

    // float y1 = m * p1.x + q - p1.y;
    // float y2 = m * p2.x + q - p2.y;

    // float area = 0.5 * (y1 + y2);

    // oFragColor = area; 
    if (vPos.y > ceil(to.y) || vPos.y < floor(from.y) || vPos.x < floor(min(from.x, to.x))) {
      oFragColor = 0.0;
      return;
    }

    float y0 = floor(vPos.y);

    if (abs(m) > 999.0) {
      float area = to.y - y0;
      oFragColor = clamp(area, 0.0, 1.0);
      return;
    }

    float x1 = floor(vPos.x);
    float x2 = x1 + 1.0;

    float y1 = (m * x1 + q) - y0;
    float y2 = (m * x2 + q) - y0;

    // float area = winding < 0.0 ? clamp(0.5 * (y1 + y2), 0.0, 1.0) : clamp(0.5 * (y1 + y2), -1.0, 0.0);
    float area = 0.5 * (y1 + y2);

    if (winding > 0.0) {
      oFragColor = area;
    } else {
      oFragColor = area - 1.0;
    }
    // float area = abs(clamp(0.5 * (y1 + y2), -1.0, 0.0));
    // float area = -0.5 * (y1 + y2);

    // oFragColor = y2 + area * 0.00000000001;
  }

)"
