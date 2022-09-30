import HistoryManager from '@editor/history';
import { vec2 } from '@math';
import Handle from '../entities/handle';

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

  tempGet() {
    return this.m_delta;
  }

  set(value: number) {
    const backup = this.m_value;

    HistoryManager.record({
      fn: () => {
        this.m_value = value;
        if (this.m_callback) this.m_callback();
      },
      undo: () => {
        this.m_value = backup;
        if (this.m_callback) this.m_callback();
      }
    });
  }

  add(delta: number) {
    HistoryManager.record({
      fn: () => {
        this.m_value += delta;
        if (this.m_callback) this.m_callback();
      },
      undo: () => {
        this.m_value -= delta;
        if (this.m_callback) this.m_callback();
      }
    });
  }

  translate(delta: number) {
    this.m_delta += delta;
    if (this.m_callback) this.m_callback();
  }

  apply() {
    const transformed = this.get();

    if (transformed !== this.m_value) {
      this.set(transformed);
      this.clear();
    }
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

  tempGet() {
    return vec2.clone(this.m_delta);
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

  set position(value: vec2) {
    this.m_position.set(value);
  }

  get staticPosition() {
    return this.m_position.staticGet();
  }

  get tempPosition() {
    return this.m_position.tempGet();
  }

  set tempPosition(value: vec2) {
    this.tempTranslate(vec2.sub(value, this.position));
  }

  translate(delta: vec2) {
    this.m_position.add(delta);
  }

  tempTranslate(delta: vec2) {
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

class UntrackedSimpleTransform implements UntrackedSimpleTransformComponent {
  private m_position: vec2;

  private m_callback: (() => void) | undefined;

  constructor(position?: vec2, callback?: () => void) {
    this.m_position = position ?? vec2.create();
    this.m_callback = callback;
  }

  get position() {
    return vec2.clone(this.m_position);
  }

  set position(value: vec2) {
    vec2.copy(this.m_position, value);
    if (this.m_callback) this.m_callback();
  }

  translate(delta: vec2) {
    vec2.add(this.m_position, delta, true);
    if (this.m_callback) this.m_callback();
  }

  get mat3() {
    return [0, 0, 0, 0, 0, 0, 0, 0, 0];
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

  set position(value: vec2) {
    this.m_position.set(value);
  }

  get staticPosition() {
    return this.m_position.staticGet();
  }

  get tempPosition() {
    return this.m_position.tempGet();
  }

  set tempPosition(value: vec2) {
    this.tempTranslate(vec2.sub(value, this.position));
  }

  translate(delta: vec2) {
    this.m_position.add(delta);
  }

  tempTranslate(delta: vec2) {
    this.m_position.translate(delta);
  }

  get rotation() {
    return this.m_rotation.get();
  }

  set rotation(value: number) {
    this.m_rotation.set(value);
  }

  get staticRotation() {
    return this.m_rotation.staticGet();
  }

  get tempRotation() {
    return this.m_rotation.tempGet();
  }

  set tempRotation(value: number) {
    this.tempRotate(value - this.rotation);
  }

  rotate(delta: number) {
    this.m_rotation.add(delta);
  }

  tempRotate(delta: number) {
    this.m_rotation.translate(delta);
  }

  get scale() {
    return this.m_scale.get();
  }

  get mat3() {
    return [0, 0, 0, 0, 0, 0, 0, 0, 0];
  }

  clear() {
    this.m_position.clear();
    this.m_rotation.clear();
  }

  apply() {
    this.m_position.apply();
    this.m_rotation.apply();
  }
}

class UntrackedTransform implements UntrackedTransformComponent {
  origin: vec2;

  private m_position: vec2;
  private m_rotation: number;
  private m_scale: vec2;

  private m_callback: (() => void) | undefined;

  constructor(position?: vec2, rotation?: number, scale?: vec2, callback?: () => void) {
    this.m_position = position ?? vec2.create();
    this.m_rotation = rotation ?? 0;
    this.m_scale = scale ?? vec2.create();

    this.m_callback = callback;
  }

  get position() {
    return vec2.clone(this.m_position);
  }

  set position(value: vec2) {
    vec2.copy(this.m_position, value);
    if (this.m_callback) this.m_callback();
  }

  translate(delta: vec2) {
    vec2.add(this.m_position, delta, true);
    if (this.m_callback) this.m_callback();
  }

  get rotation() {
    return this.m_rotation;
  }

  set rotation(value: number) {
    this.m_rotation = value;
    if (this.m_callback) this.m_callback();
  }

  rotate(delta: number) {
    this.m_rotation += delta;
    if (this.m_callback) this.m_callback();
  }

  get scale() {
    return vec2.clone(this.m_scale);
  }

  get mat3() {
    return [0, 0, 0, 0, 0, 0, 0, 0, 0];
  }
}

class VertexTransform implements VertexTransformComponent {
  private m_vertex: VertexEntity;

  constructor(vertex: VertexEntity) {
    this.m_vertex = vertex;
  }

  private get m_position(): SimpleTransformComponent {
    return this.m_vertex.position.transform;
  }

  private get m_left(): SimpleTransformComponent | undefined {
    return this.m_vertex.left?.transform;
  }

  private get m_right(): SimpleTransformComponent | undefined {
    return this.m_vertex.right?.transform;
  }

  get position(): vec2 {
    return this.m_position.position;
  }

  set position(value: vec2) {
    this.m_position.position = value;
  }

  get staticPosition(): vec2 {
    return this.m_position.staticPosition;
  }

  get tempPosition() {
    return this.m_position.tempPosition;
  }

  set tempPosition(value: vec2) {
    this.m_position.tempPosition = value;
  }

  translate(delta: vec2) {
    this.m_position.translate(delta);
  }

  tempTranslate(delta: vec2): void {
    this.m_position.tempTranslate(delta);
  }

  get left(): vec2 {
    return this.m_left ? this.m_left.position : vec2.create();
  }

  set left(value: vec2) {
    if (this.m_left) {
      this.m_left.position = value;
    } else {
      this.m_vertex.left = new Handle({ position: value, type: 'bezier', parent: this.m_vertex });
    }
  }

  get staticLeft(): vec2 {
    return this.m_left ? this.m_left.staticPosition : vec2.create();
  }

  get tempLeft() {
    return this.m_left ? this.m_left.tempPosition : vec2.create();
  }

  set tempLeft(value: vec2) {
    if (this.m_left) {
      this.m_left.tempPosition = value;
    }
  }

  translateLeft(delta: vec2) {
    this.m_left?.translate(delta);
  }

  tempTranslateLeft(delta: vec2, lockMirror: boolean = false): void {
    this.m_left?.tempTranslate(delta);

    if (!lockMirror && this.m_right) {
      const direction = vec2.unit(vec2.neg(this.left));
      if (!vec2.equals(direction, [0, 0])) {
        this.tempTranslateRight(
          vec2.sub(vec2.mul(direction, vec2.len(this.right!)), this.right),
          true
        );
      }
    }
  }

  get right(): vec2 {
    return this.m_right ? this.m_right.position : vec2.create();
  }

  set right(value: vec2) {
    if (this.m_right) {
      this.m_right.position = value;
    } else {
      this.m_vertex.right = new Handle({ position: value, type: 'bezier', parent: this.m_vertex });
    }
  }

  get staticRight(): vec2 {
    return this.m_right ? this.m_right.staticPosition : vec2.create();
  }

  get tempRight() {
    return this.m_right ? this.m_right.tempPosition : vec2.create();
  }

  set tempRight(value: vec2) {
    if (this.m_right) {
      this.m_right.tempPosition = value;
    }
  }

  translateRight(delta: vec2) {
    this.m_right?.translate(delta);
  }

  tempTranslateRight(delta: vec2, lockMirror: boolean = false): void {
    this.m_right?.tempTranslate(delta);

    if (!lockMirror && this.m_left) {
      const direction = vec2.unit(vec2.neg(this.right));
      if (!vec2.equals(direction, [0, 0])) {
        this.tempTranslateLeft(
          vec2.sub(vec2.mul(direction, vec2.len(this.left!)), this.left),
          true
        );
      }
    }
  }

  apply(): void {
    this.m_position.apply();
    this.m_left?.apply();
    this.m_right?.apply();
  }

  clear(): void {
    this.m_position.clear();
    this.m_left?.clear();
    this.m_right?.clear();
  }
}

export {
  SimpleTransform,
  Transform,
  UntrackedSimpleTransform,
  UntrackedTransform,
  VertexTransform
};
