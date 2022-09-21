export * as vec2 from './vec2';
export * as vec3 from './vec3';
export * as vec4 from './vec4';
export * as mat4 from './mat4';

export * from './math';

export function vectorMatrixCallback({
  value,
  int,
  float,
  vec2,
  vec3,
  vec4,
  mat2,
  mat3,
  mat4
}: {
  value: VectorOrMatrix;
  int?(...args: any): void;
  float?(...args: any): void;
  vec2?(...args: any): void;
  vec3?(...args: any): void;
  vec4?(...args: any): void;
  mat2?(...args: any): void;
  mat3?(...args: any): void;
  mat4?(...args: any): void;
}): (...args: any) => void {
  if (typeof value === 'object') {
    if (value.length === 2 && vec2) return vec2;
    else if (value.length === 3 && vec3) return vec3;
    else if (value.length === 4 && vec4) return vec4;
    else if (value.length === 4 && mat2) return mat2;
    else if (value.length === 9 && mat3) return mat3;
    else if (value.length === 16 && mat4) return mat4;
  } else if (typeof value === 'number') {
    if (value - Math.floor(value) === 0 && int) return int;
    else if (float) return float;
  }
  return () => {};
}
