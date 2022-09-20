import HistoryManager from '@editor/history';
import { vec2 } from '@math';

class TransformNumberValue implements TransformComponentValue<number> {
  private m_value;
  private m_delta: number = 0;

  private m_callback: (() => void) | undefined;

  constructor(value?: number, callback?: () => void) {
    this.m_value = value ?? 0;
    this.m_callback = callback;
  }

  get() {
    return this.m_value + this.m_delta;
  }

  staticGet() {
    return this.m_value;
  }

  set(value: number) {
    this.m_value = value;
    if (this.m_callback) this.m_callback();
  }

  add(delta: number) {
    this.m_value += delta;
    if (this.m_callback) this.m_callback();
  }

  translate(delta: number) {
    this.m_delta += delta;
    if (this.m_callback) this.m_callback();
  }

  apply() {
    this.m_value += this.m_delta;
    this.clear();
  }

  clear() {
    this.m_delta = 0;
    if (this.m_callback) this.m_callback();
  }
}

class TransformVec2Value implements TransformComponentValue<vec2> {
  private m_value: vec2;
  private m_delta: vec2 = vec2.create();

  private m_callback: (() => void) | undefined;

  constructor(value?: vec2, callback?: () => void) {
    this.m_value = value ? vec2.clone(value) : vec2.create();
    this.m_callback = callback;
  }

  get() {
    return vec2.add(this.m_value, this.m_delta);
  }

  staticGet() {
    return vec2.clone(this.m_value);
  }

  set(value: vec2) {
    const backup = vec2.clone(this.m_value);

    HistoryManager.record({
      fn: () => {
        vec2.copy(this.m_value, value);
        if (this.m_callback) this.m_callback();
      },
      undo: () => {
        vec2.copy(this.m_value, backup);
        if (this.m_callback) this.m_callback();
      }
    });
  }

  add(delta: vec2) {
    HistoryManager.record({
      fn: () => {
        vec2.add(this.m_value, delta, true);
        if (this.m_callback) this.m_callback();
      },
      undo: () => {
        vec2.sub(this.m_value, delta, true);
        if (this.m_callback) this.m_callback();
      }
    });
  }

  translate(delta: vec2) {
    vec2.add(this.m_delta, delta, true);
    if (this.m_callback) this.m_callback();
  }

  apply() {
    const transformed = this.get();

    if (!vec2.equals(transformed, this.m_value)) {
      this.set(transformed);
      this.clear();
    }
  }

  clear() {
    vec2.copy(this.m_delta, [0, 0]);
    if (this.m_callback) this.m_callback();
  }
}

class SimpleTransform implements SimpleTransformComponent {
  private m_position: TransformVec2Value;

  constructor(position?: vec2, callback?: () => void) {
    this.m_position = new TransformVec2Value(position, callback);
  }

  get position() {
    return this.m_position.get();
  }

  get staticPosition() {
    return this.m_position.staticGet();
  }

  set position(value: vec2) {
    this.m_position.set(value);
  }

  set translation(value: vec2) {
    this.translate(vec2.sub(value, this.position));
  }

  move(delta: vec2) {
    this.m_position.add(delta);
  }

  translate(delta: vec2) {
    this.m_position.translate(delta);
  }

  get mat3() {
    return [0, 0, 0, 0, 0, 0, 0, 0, 0];
  }

  clear() {
    this.m_position.clear();
  }

  apply() {
    this.m_position.apply();
  }
}

class Transform implements TransformComponent {
  origin: vec2;

  private m_position: TransformVec2Value;
  private m_rotation: TransformNumberValue;
  private m_scale: TransformVec2Value;

  constructor(position?: vec2, rotation?: number, scale?: vec2, callback?: () => void) {
    this.m_position = new TransformVec2Value(position, callback);
    this.m_rotation = new TransformNumberValue(rotation, callback);
    this.m_scale = new TransformVec2Value(scale, callback);
  }

  get position() {
    return this.m_position.get();
  }

  get staticPosition() {
    return this.m_position.staticGet();
  }

  set position(value: vec2) {
    this.m_position.set(value);
  }

  set translation(value: vec2) {
    this.translate(vec2.sub(value, this.position));
  }

  move(delta: vec2) {
    this.m_position.add(delta);
  }

  translate(delta: vec2) {
    this.m_position.translate(delta);
  }

  get rotation() {
    return this.m_rotation.get();
  }

  get scale() {
    return this.m_scale.get();
  }

  get mat3() {
    return [0, 0, 0, 0, 0, 0, 0, 0, 0];
  }

  clear() {
    this.m_position.clear();
  }

  apply() {
    this.m_position.apply();
  }
}

export { SimpleTransform, Transform };
