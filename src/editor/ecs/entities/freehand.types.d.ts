interface FreehandEntity extends Entity {
  readonly type: 'freehand';
  readonly selectable: true;
  readonly transform: TransformComponent;

  forEach(callback: (point: SimpleTransformComponent) => void): void;
  add(point: vec2, pressure?: number): void;
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
