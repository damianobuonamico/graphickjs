R"(

  precision highp float;
  precision highp sampler2D;

  uniform sampler2D uPathsTexture;
  uniform int uPathsTextureSize;
  
  in float vPathIndex;
  in vec4 vColor;
  in vec2 vCoords;

  out vec4 oFragColor;

  vec2 index_to_coords(int index) {
    return vec2(
      float(index % uPathsTextureSize) / float(uPathsTextureSize),
      float(index / uPathsTextureSize) / float(uPathsTextureSize)
    );
  }

  float x_intersect(vec2 p0, vec2 p3, float y) {
    float m = (p3.y - p0.y) / (p3.x - p0.x);
    float b = p0.y - m * p0.x;
    return (y - b) / m;
  }

  float y_intersect(vec2 p0, vec2 p3, float x) {
    float m = (p3.y - p0.y) / (p3.x - p0.x);
    float b = p0.y - m * p0.x;
    return m * x + b;
  }

  void main() {
    int index = int(vPathIndex);
    vec2 tex_coords = index_to_coords(index);

    int n = int(texture(uPathsTexture, tex_coords).r);

    float alpha = 0.0;
    // int winding = 0;

    for (int i = 0; i < n; i++) {
      vec2 p0_x_coords = index_to_coords(index + i * 4 + 1);
      vec2 p0_y_coords = index_to_coords(index + i * 4 + 2);
      vec2 p3_x_coords = index_to_coords(index + i * 4 + 3);
      vec2 p3_y_coords = index_to_coords(index + i * 4 + 4);

      vec2 p0 = vec2(texture(uPathsTexture, p0_x_coords).r, texture(uPathsTexture, p0_y_coords).r);
      vec2 p3 = vec2(texture(uPathsTexture, p3_x_coords).r, texture(uPathsTexture, p3_y_coords).r);

      // float min_x = min(p0.x, p3.x);
      // float max_x = max(p0.x, p3.x);
      float min_y = min(p0.y, p3.y);
      float max_y = max(p0.y, p3.y);

      float y0 = floor(vCoords.y);
      float y1 = y0 + 1.0;
      float x0 = floor(vCoords.x);
      float x1 = x0 + 1.0;

      if (min_y >= y1 || max_y <= y0/* || min_x >= x1*/) {
        continue;
      }

      float area = 0.0;
      float cover = 0.0;

      // Check if segment is almost vertical
      if (abs(p0.x - p3.x) < 0.1 ) {
        // Treating segment as vertical, horizontal segments are discarded in CPU preprocessing
        if (p0.x > x1) {
          // Segment is on the right side of the pixel
          continue;
        }

        if (p0.x < x0) {
          cover += clamp(p3.y, y0, y1) - clamp(p0.y, y0, y1);
          // cover += clamp(p3.y - p0.y, -1.0, 1.0);
        } else {
          area += (clamp(p3.y, y0, y1) - clamp(p0.y, y0, y1)) * (x1 - p0.x);
          // area += clamp(p3.y - p0.y, -1.0, 1.0) * (x1 - p0.x);
        }

        // if (p0.x < x0) {
        //   cover += clamp(p3.y - p0.y, -1.0, 1.0);
        // } else {
        //   area += clamp(p3.y - p0.y, -1.0, 1.0) * (x1 - p0.x);
        // }

        alpha += area + cover;

        continue;
      }

      // float y = floor(vCoords.y) + 0.5;

      // float x_inter = x_intersect(p0, p3, y);

      // if (x_inter < vCoords.x) {
      //   if (p0.y > p3.y) {
      //     winding++;
      //   } else {
      //     winding--;
      //   }
      // }

      // Clip segment to pixel
      if (p0.y < y0) {
        p0 = vec2(x_intersect(p0, p3, y0), y0);
      } else if (p0.y > y1) {
        p0 = vec2(x_intersect(p0, p3, y1), y1);
      }

      if (p3.y < y0) {
        p3 = vec2(x_intersect(p0, p3, y0), y0);
      } else if (p3.y > y1) {
        p3 = vec2(x_intersect(p0, p3, y1), y1);
      }

      float min_x = min(p0.x, p3.x);
      float max_x = max(p0.x, p3.x);

      if (min_x >= x1) {
        continue;
      }

      if (p0.x > x1) {
        p0 = vec2(x1, y_intersect(p0, p3, x1));
      } else if (p3.x > x1) {
        p3 = vec2(x1, y_intersect(p0, p3, x1));
      }

      // Calculate area and cover values
      if (p0.x < x0) {
        if (p3.x < x0) {
          // Both points are on the left side of the pixel
          // cover += p0.y - p3.y;
          cover += p3.y - p0.y;
        } else {
          // p0 is on the left side of the pixel, p3 is on the right side, clipping
          vec2 p0_a = vec2(x0, y_intersect(p0, p3, x0));
          // area += 0.5 * (1.0 + x1 - p3.x) * (p0_a.y - p3.y);
          // cover += p0.y - p0_a.y;
          area += 0.5 * (1.0 + x1 - p3.x) * (p3.y - p0_a.y);
          cover += p0_a.y - p0.y;
        }
      } else if (p3.x < x0) {
        // p3 is on the left side of the pixel, p0 is on the right side, clipping
        vec2 p3_a = vec2(x0, y_intersect(p0, p3, x0));
        // area += 0.5 * (1.0 + x1 - p0.x) * (p0.y - p3_a.y);
        // cover += p3_a.y - p3.y;
        area += 0.5 * (1.0 + x1 - p0.x) * (p3_a.y - p0.y);
        cover += p3.y - p3_a.y;
      } else {
        // Both points are on the right side of the pixel
        // area += 0.5 * (x1 - p0.x + x1 - p3.x) * (p0.y - p3.y);
        area += 0.5 * (x1 - p0.x + x1 - p3.x) * (p3.y - p0.y);
      }

      alpha += area + cover;
    }

    // oFragColor = vec4(vColor.rgb, winding != 0 ? 1.0 : 0.0);
    oFragColor = vec4(vColor.rgb, min(abs(alpha), 1.0));
  } 

  void mainy() {
    int index = int(vPathIndex);
    vec2 tex_coords = index_to_coords(index);

    int n = int(texture(uPathsTexture, tex_coords).r);

    float alpha = 0.0;
    int winding = 0;

    for (int i = 0; i < n; i++) {
      vec2 p0_x_coords = index_to_coords(index + i * 4 + 1);
      vec2 p0_y_coords = index_to_coords(index + i * 4 + 2);
      vec2 p3_x_coords = index_to_coords(index + i * 4 + 3);
      vec2 p3_y_coords = index_to_coords(index + i * 4 + 4);

      vec2 p0 = vec2(texture(uPathsTexture, p0_x_coords).r, texture(uPathsTexture, p0_y_coords).r);
      vec2 p3 = vec2(texture(uPathsTexture, p3_x_coords).r, texture(uPathsTexture, p3_y_coords).r);

      float min_x = min(p0.x, p3.x);
      float max_x = max(p0.x, p3.x);
      float min_y = min(p0.y, p3.y);
      float max_y = max(p0.y, p3.y);

      float x0 = floor(vCoords.x);
      float x1 = x0 + 1.0;
      float y0 = floor(vCoords.y);
      float y1 = y0 + 1.0;

      if (min_y >= y0 || max_y <= y0 || min_x >= x1) {
        continue;
      }
      
      float x_t = x_intersect(p0, p3, y0);
      float x_b = x_intersect(p0, p3, y1);

      if (x_t >= x1 && x_b >= x1) {
        continue;
      }

      if (x_t < x0 && x_b < x0) {
        winding++;
        continue;
      }

      // calculate edge coverage in pixel
      float y_r = y_intersect(p0, p3, x1);
      float y_l = y_intersect(p0, p3, x0);

      float area = 0.0;

      if ((y_r > y1 || y_r < y0) && (y_l > y1 || y_l < y0)) {
        // Trapezoid case
        area += 0.5 * (x1 - x_t + x1 - x_b);
  
      }

      if (winding % 2 == 0) {
        alpha -= area;
      } else {
        alpha += area;
      }

      // bool flip = false;

      // if (p0.y < p3.y) {
      //   vec2 tmp = p0;
      //   p0 = p3;
      //   p3 = tmp;
      //   flip = true;
      // }

      // float x_b = x_intersect(p0, p3, y1);
      
      // if (p0.y > y1) {
      //   p0 = vec2(x_b, y1);
      // }

      // float x_t = x_intersect(p0, p3, y0);

      // if (p3.y < y0) {
      //   p3 = vec2(x_t, y0);
      // }

      // if (max(x_t, x_b) < x0) {
      //   if (flip) alpha -= 1.0;
      //   else alpha += 1.0;
      //   continue;
      // }
      
      // float y_r = y_intersect(p0, p3, x1);

      // if (p0.x > x1) {
      //   p0 = vec2(x1, y_r);
      // } else if (p3.x > x1) {
      //   p3 = vec2(x1, y_r);
      // }
      
      // float y_l = y_intersect(p0, p3, x0);

      // if (p0.x < x0) {
      //   p0 = vec2(x0, y_l);
      // } else if (p3.x < x0) {
      //   p3 = vec2(x0, y_l);
      // }

      // float cover = p3.y - p0.y;
      // float area = 0.5 * cover * ((x1 - p0.x) + (x1 - p3.x));
    
      // if (p0.x > p3.x) {
      //   area = 0.5 * (2.0 * x1 - p0.x - max(p3.x, x0)) * (clamp(p0.y, 0.0, 1.0) - y_l);
      //   cover = min(p3.y, y1) - y_l;
      // } else {
      //   area = 0.5 * (2.0 * x1 - p3.x - max(p0.x, x0)) * (clamp(p3.y, 0.0, 1.0) - y_l);
      //   cover = min(p0.y, y1) - y_l; 
      // }



      // TODO: fix no intersection case

      // TODO: fix trapezoid case
      // float area = 0.5 * (p3.y - y_l);
      // float cover = y_l - y0;
      // float area = 0.5 * (2.0 * x1 - clamp(x_t, x0, x1) - clamp(x_b, x0, x1)) * (clamp(y_r, y0, y1) - clamp(y_l, y0, y1));
      // float cover = y1 - clamp(y_l, y0, y1);



      // if (flip) {
      //   alpha -= area + cover;
      // } else {
      //   alpha += area + cover;
      // }
    }

    oFragColor = vec4(vColor.rgb, clamp((winding % 2 == 0 ? 0.0 : 1.0) + alpha, 0.0, 1.0));
  }

  void mainn() {
    int index = int(vPathIndex);
    vec2 tex_coords = index_to_coords(index);

    int n = int(texture(uPathsTexture, tex_coords).r);

    int winding = 0;

    float alpha = 0.0;
    float accum = 0.0;

    for (int i = 0; i < n; i++) {
      vec2 p0_x_coords = index_to_coords(index + i * 4 + 1);
      vec2 p0_y_coords = index_to_coords(index + i * 4 + 2);
      vec2 p3_x_coords = index_to_coords(index + i * 4 + 3);
      vec2 p3_y_coords = index_to_coords(index + i * 4 + 4);

      vec2 p0 = vec2(texture(uPathsTexture, p0_x_coords).r, texture(uPathsTexture, p0_y_coords).r);
      vec2 p3 = vec2(texture(uPathsTexture, p3_x_coords).r, texture(uPathsTexture, p3_y_coords).r);

      float min_x = min(p0.x, p3.x);
      float min_y = min(p0.y, p3.y);
      float max_y = max(p0.y, p3.y);

      float x0 = floor(vCoords.x);
      float x1 = x0 + 1.0;
      float y0 = floor(vCoords.y);
      float y1 = y0 + 1.0;
  
      if (min_y >= y0 || max_y <= y0 || min_x >= x1) {
        continue;
      }

      float x_t = x_intersect(p0, p3, y0);
      float x_b = x_intersect(p0, p3, y1);

      if (x_t >= x1 && x_b >= x1) continue;

      float y_l = y_intersect(p0, p3, x0);
      float y_r = y_intersect(p0, p3, x1);

      // float min_x_ = min(x_t, x_b);
      // float max_x_ = max(x_t, x_b);
      // float min_y_ = min(y_l, y_r);
      // float max_y_ = max(y_l, y_r);

      // float area = 0.5 * (2.0 * x1 - clamp(x_t, x0, x1) - clamp(x_b, x0, x1)) * (clamp(y_r, y0, y1) - clamp(y_l, y0, y1));
      // float cover = y1 - clamp(y_l, y0, y1);

      //   cover *= -1.0;
      // if (p0.y > p3.y) {
      // }

      // float area = 0.5 * (2.0 * x1 - max(min_x_, x0) - min(max_x_, x1)) * (min(max_y_, y1) - max(min_y_, y0));
      // alpha += area + cover;

      
      // accum += height;
    }

    oFragColor = vec4(vColor.rgb, min(abs(alpha), 1.0));
  }

)"
