interface FreehandEntity extends Entity {
  readonly type: 'freehand';
  readonly selectable: true;
  readonly transform: TransformComponent;
  readonly geometry: [Float32Array, number[]];

  forEach(callback: (point: SimpleTransformComponent) => void): void;
  add(point: vec2, pressure?: number): void;
  erase(position: vec2, radius: number): void;
}

interface FreehandOptions {
  position: vec2;
  rotation?: number;
  points: vec3[];
  color?: string;
}

interface FreehandObject extends GenericEntityObject {
  position: vec2;
  rotation?: number;
  points: vec3[];
}
