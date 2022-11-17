declare type vec2 = [number, number];
declare type vec3 = [number, number, number];
declare type vec4 = [number, number, number, number];

// prettier-ignore
declare type mat3 = [
  number, number, number,
  number, number, number,
  number, number, number,
];

// prettier-ignore
declare type mat4 = [
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
];

declare type ReadonlyVec2 = readonly [number, number];
declare type ReadonlyVec3 = readonly [number, number, number];
declare type ReadonlyVec4 = readonly [number, number, number, number];

// prettier-ignore
declare type ReadonlyMat3 = [
  number, number, number,
  number, number, number,
  number, number, number,
];

// prettier-ignore
declare type ReadonlyMat4 = readonly [
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
];

declare type VectorOrMatrix = number | vec2 | vec3 | vec4 | mat3 | mat4;

type Box = [vec2, vec2];
