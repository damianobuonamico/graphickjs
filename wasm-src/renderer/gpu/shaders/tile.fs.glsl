R"(

  precision highp float;
  precision highp sampler2D;

  //uniform int uTileSize;
  uniform sampler2D uSegmentsTexture;

  in vec4 vColor;
  in vec2 vPosition;
  in vec2 vMaskCoords;
  
  flat in int vSegmentIndex;

  out vec4 oFragColor;

  #define SEGMENTS_TEXTURE_SIZE 2048
  // #define USE_ANTIALIASING

  vec2 segment_coords(int segment_index) {
    return vec2((float(segment_index % SEGMENTS_TEXTURE_SIZE) + 0.5) / float(SEGMENTS_TEXTURE_SIZE), (float(segment_index / SEGMENTS_TEXTURE_SIZE) + 0.5) / float(SEGMENTS_TEXTURE_SIZE));
  }

  void main() {
    float aalpha = texture(uSegmentsTexture, vMaskCoords).r;
    oFragColor = vec4(vColor.rgb, vColor.a * aalpha);
    return;

//     int segments = int(texture(uSegmentsTexture, segment_coords(vSegmentIndex)).r * 255.0);
    
//     // oFragColor = vec4(vColor.r, 0.0, 0.0, float(segments) / 3.0);
//     // return;

//     if (segments == 0) discard;

//     int winding = 0;
//     float alpha = 0.0;

//     for (int i = 0; i < segments; i++) {
//       vec2 from = float(uTileSize) * vec2(
//         texture(uSegmentsTexture, segment_coords(vSegmentIndex + i * 4 + 1)).r,
//         texture(uSegmentsTexture, segment_coords(vSegmentIndex + i * 4 + 2)).r
//       );
//       vec2 to = float(uTileSize) * vec2(
//         texture(uSegmentsTexture, segment_coords(vSegmentIndex + i * 4 + 3)).r,
//         texture(uSegmentsTexture, segment_coords(vSegmentIndex + i * 4 + 4)).r
//       );

//       if (max(from.y, to.y) < vPosition.y || min(from.y, to.y) > vPosition.y) continue;

//       if (abs(from.x - to.x) < 0.0001) {
//         if (from.x <= vPosition.x) {
//           winding++;
//         }
//         continue;
//       }

//       float m = (to.y - from.y) / (to.x - from.x);
//       float q = from.y - m * from.x;

//       float y0 = floor(vPosition.y + 0.5);
//       float x0 = ceil(vPosition.x + 0.5);

//       float x1 = x0 - (y0 - q) / m;
//       float x2 = x0 - (y0 + 1.0 - q) / m;
// #ifdef USE_ANTIALIASING
//       if (min(x1, x2) >= 1.0) {
//         winding++;
//       } else if (max(x1, x2) <= 0.0) {
//         // continue;
//       } else {
//         float area = 0.0;

//         if (x1 > 1.0) {
//           float y1 = m * (x0 - 1.0) + q - y0;
          
//           if (x2 < 0.0) {
//             float y2 = m * x0 + q - y0;
//             area = 1.0 - 0.5 * (y1 + y2);
//           } else {
//             area = 0.5 * (x1 + x2) + (1.0 - y1);
//           }
//         } else if (x1 < 0.0) {
//           float y1 = m * x0 + q - y0;
          
//           if (x2 > 1.0) {
//             float y2 = m * (x0 - 1.0) + q - y0;
//             area = 0.5 * (y1 + y2);
//           } else {
//             area = 0.5 * (x1 + x2);
//           }
//         } else {
//           if (x2 < 0.0) {
//             float y2 = m * x0 + q - y0;
//             area = 0.5 * x1 * y2;
//           } else if (x2 > 1.0) {
//             float y2 = m * (x0 - 1.0) + q - y0;
//             area = 0.5 * (1.0 + x0) * y2 + (1.0 - y2);
//           } else {
//             area = 0.5 * (x1 + x2);          
//           }
//         }

//         alpha += area;
//       }      
// #else
//       if (min(x1, x2) >= 1.0) {
//         winding++;
//       } else if (max(x1, x2) <= 0.0) {
//         // continue;
//       } else {
//         // antialiasing
//       }
// #endif
//     }

//     if (winding % 2 == 0) {
//       if (alpha == 0.0) discard;
//       if (alpha < 0.0) {
//         oFragColor = vec4(vColor.rgb, vColor.a * abs(alpha));
//       } else {
//         oFragColor = vec4(vColor.rgb, vColor.a * alpha);
//       }
//     } else {
//       oFragColor = vColor;
//     }
  }

)"
