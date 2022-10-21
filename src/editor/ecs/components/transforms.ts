import HistoryManager from '@/editor/history';
import { equals, mat3, vec2 } from '@/math';

export class FloatValue {
  private m_value: number;
  private m_delta = 0;

  constructor(value: number = 0) {
    this.m_value = value;
  }

  get() {
    return this.m_value + this.m_delta;
  }

  staticGet() {
    return this.m_value;
  }

  set(value: number) {
    const backup = this.m_value;
    if (equals(this.m_value, value)) return;

    HistoryManager.record({
      fn: () => {
        this.m_value = value;
      },
      undo: () => {
        this.m_value = backup;
      }
    });
  }

  add(delta: number) {
    if (equals(delta, 0)) return;

    HistoryManager.record({
      fn: () => {
        this.m_value += delta;
      },
      undo: () => {
        this.m_value -= delta;
      }
    });
  }

  tempAdd(value: number) {
    this.m_delta += value;
  }

  apply() {
    const transformed = this.get();

    if (!equals(transformed, this.m_value)) {
      this.set(transformed);
      this.clear();
    }
  }

  clear() {
    this.m_delta = 0;
  }
}

export class Vec2Value {
  private m_value: vec2;
  private m_delta = vec2.create();

  constructor(value: vec2 = [0, 0]) {
    this.m_value = vec2.clone(value);
  }

  get() {
    return vec2.add(this.m_value, this.m_delta);
  }

  getStatic() {
    return vec2.clone(this.m_value);
  }

  getDelta() {
    return vec2.clone(this.m_delta);
  }

  set(value: vec2) {
    const backup = vec2.clone(this.m_value);
    if (vec2.equals(backup, value)) return;

    HistoryManager.record({
      fn: () => {
        vec2.copy(this.m_value, value);
      },
      undo: () => {
        vec2.copy(this.m_value, backup);
      }
    });
  }

  add(delta: vec2) {
    if (vec2.equals(delta, [0, 0])) return;

    HistoryManager.record({
      fn: () => {
        vec2.add(this.m_value, delta, this.m_value);
      },
      undo: () => {
        vec2.sub(this.m_value, delta, this.m_value);
      }
    });
  }

  tempAdd(value: vec2) {
    vec2.add(this.m_delta, value, this.m_delta);
  }

  apply() {
    const transformed = this.get();

    if (!vec2.equals(transformed, this.m_value)) {
      this.set(transformed);
      this.clear();
    }
  }

  clear() {
    vec2.zero(this.m_delta);
  }
}

export class RectTransform {
  private m_translation: Vec2Value;
  private m_rotation = new FloatValue();
  private m_scale = vec2.fromValues(1, 1);
  private m_reflection: Vec2Value;

  private m_origin = vec2.create();
  private m_size = vec2.create();

  constructor(
    position: vec2 = [0, 0],
    rotation: number = 0,
    size: vec2 = [0, 0],
    reflect: vec2 = [1, 1]
  ) {
    this.m_translation = new Vec2Value(position);
    this.m_rotation = new FloatValue(rotation);
    this.m_size = vec2.clone(size);
    this.m_reflection = new Vec2Value(reflect);
  }

  get position() {
    return this.m_translation.get();
  }

  get rotation() {
    return this.m_rotation.get();
  }

  get scale() {
    return vec2.clone(this.m_scale);
  }

  get reflection() {
    return this.m_reflection.get();
  }

  get size() {
    return vec2.clone(this.m_size);
  }

  get origin() {
    return vec2.clone(this.m_origin);
  }

  get center() {
    return this.getCenter();
  }

  get staticPosition() {
    return this.m_translation.getStatic();
  }

  get tempPosition() {
    return this.m_translation.getDelta();
  }

  get staticBoundingBox(): Box {
    const position = this.m_translation.getStatic();
    return [position, vec2.add(position, this.m_size)];
  }

  get unrotatedBoundingBox(): Box {
    const position = this.m_translation.getStatic();
    const box: Box = [position, vec2.add(position, this.m_size)];

    if (!vec2.exactEquals(this.m_scale, [1, 1])) {
      vec2.scale(box[0], this.origin, this.m_scale, box[0]);
      vec2.scale(box[1], this.origin, this.m_scale, box[1]);
    }

    const delta = this.m_translation.getDelta();

    vec2.add(box[0], delta, box[0]);
    vec2.add(box[1], delta, box[1]);

    return [vec2.min(box[0], box[1]), vec2.max(box[0], box[1])];
  }

  get rotatedBoundingBox(): [vec2, vec2, vec2, vec2] {
    const box = this.unrotatedBoundingBox;
    const angle = this.m_rotation.get();
    const center = this.center;

    return [
      vec2.rotate(box[0], center, angle),
      vec2.rotate([box[1][0], box[0][1]], center, angle),
      vec2.rotate(box[1], center, angle),
      vec2.rotate([box[0][0], box[1][1]], center, angle)
    ];
  }

  get boundingBox(): Box {
    if (this.m_rotation.get() === 0) return this.unrotatedBoundingBox;

    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    this.rotatedBoundingBox.forEach((vertex) => {
      vec2.min(min, vertex, min);
      vec2.max(max, vertex, max);
    });

    return [min, max];
  }

  get mat3() {
    const position = this.m_translation.get();
    const staticPosition = this.m_translation.getStatic();

    const center = this.center;
    const origin = this.origin;

    const rotation = this.m_rotation.get();

    // Translate
    const matrix = mat3.fromTranslation(position);

    // Rotate
    mat3.translate(matrix, vec2.sub(center, staticPosition), matrix);
    mat3.rotate(matrix, rotation, matrix);
    mat3.translate(matrix, vec2.sub(staticPosition, center), matrix);

    // Scale
    mat3.translate(matrix, vec2.sub(origin, staticPosition), matrix);
    mat3.scale(matrix, this.m_scale, matrix);
    mat3.translate(matrix, vec2.sub(staticPosition, origin), matrix);

    // Reflect
    mat3.translate(matrix, vec2.sub(center, staticPosition), matrix);
    mat3.scale(matrix, this.m_reflection.get(), matrix);
    mat3.translate(matrix, vec2.sub(staticPosition, center), matrix);

    return matrix;
  }

  set position(value: vec2) {
    this.m_translation.set(value);
  }

  set size(value: vec2) {
    const backup = vec2.clone(this.m_size);
    if (vec2.equals(backup, value)) return;

    HistoryManager.record({
      fn: () => {
        vec2.copy(this.m_size, value);
      },
      undo: () => {
        vec2.copy(this.m_size, backup);
      }
    });
  }

  set origin(value: vec2) {
    vec2.rotate(value, this.center, -this.m_rotation.get(), this.m_origin);
  }

  private getCenter(position: vec2 = this.m_translation.getStatic(), size: vec2 = this.m_size) {
    const semiSize = vec2.divS(size, 2);
    return vec2.add(position, semiSize, semiSize);
  }

  tempTranslate(delta: vec2) {
    this.m_translation.tempAdd(delta);
  }

  tempRotate(delta: number) {
    this.m_rotation.tempAdd(delta);
  }

  tempScale(magnitude: vec2) {
    vec2.copy(this.m_scale, magnitude);
  }

  apply() {
    this.m_translation.apply();
    this.m_rotation.apply();

    const box = this.unrotatedBoundingBox;
    const size = vec2.sub(box[1], box[0]);

    const position = vec2.rotate([0, 0], this.getCenter(), this.m_rotation.get());
    vec2.sub(
      position,
      vec2.rotate([0, 0], this.getCenter(box[0], size), this.m_rotation.get()),
      position
    );
    vec2.add(position, box[0], position);

    const reflection = this.m_reflection.get();
    if (this.m_scale[0] < 0) reflection[0] = reflection[0] === 1 ? -1 : 1;
    if (this.m_scale[1] < 0) reflection[1] = reflection[1] === 1 ? -1 : 1;
    this.m_reflection.set(reflection);

    this.m_translation.set(position);
    this.size = size;
    vec2.set(this.m_scale, 1, 1);
  }

  clear() {
    this.m_translation.clear();
    this.m_rotation.clear();
    vec2.set(this.m_scale, 1, 1);
  }
}
