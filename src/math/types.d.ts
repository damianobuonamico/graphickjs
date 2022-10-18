interface IndexedCollection extends Iterable<number> {
  readonly length: number;
  [index: number]: number;
}

declare type vec2 = [number, number];
declare type vec3 = [number, number, number];
declare type vec4 = [number, number, number, number] | IndexedCollection;

// prettier-ignore
declare type mat3 = [
  number, number, number,
  number, number, number,
  number, number, number,
] | IndexedCollection;

// prettier-ignore
declare type mat4 = [
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
] | IndexedCollection;

declare type ReadonlyVec2 = readonly [number, number];
declare type ReadonlyVec3 = readonly [number, number, number];
declare type ReadonlyVec4 = readonly [number, number, number, number] | IndexedCollection;

// prettier-ignore
declare type ReadonlyMat3 = [
  number, number, number,
  number, number, number,
  number, number, number,
] | IndexedCollection;

// prettier-ignore
declare type ReadonlyMat4 = readonly [
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
] | IndexedCollection;

declare type VectorOrMatrix = number | vec2 | vec3 | vec4;

type Box = [vec2, vec2];
