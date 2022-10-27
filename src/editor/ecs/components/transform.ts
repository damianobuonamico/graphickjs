import HistoryManager from '@/editor/history';
import { equals, mat3, vec2 } from '@/math';
import { Cache } from '@/editor/ecs/components/cache';
import Debugger from '@/utils/debugger';
import Handle from '../entities/handle';

export class FloatValue implements TransformComponentValue<number> {
  private m_value: number;
  private m_delta = 0;

  constructor(value: number = 0) {
    this.m_value = value;
  }

  get() {
    return this.m_value + this.m_delta;
  }

  getStatic() {
    return this.m_value;
  }

  getDelta() {
    return this.m_delta;
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

export class CachedFloatValue implements TransformComponentValue<number> {
  private m_value: number;
  private m_delta = 0;
  private m_cache: Cache;

  constructor(cache: Cache, value: number = 0) {
    this.m_value = value;
    this.m_cache = cache;
  }

  get() {
    return this.m_value + this.m_delta;
  }

  getStatic() {
    return this.m_value;
  }

  getDelta() {
    return this.m_delta;
  }

  set(value: number) {
    const backup = this.m_value;
    if (equals(this.m_value, value)) return;

    HistoryManager.record({
      fn: () => {
        this.m_value = value;
        this.m_cache.pause = true;
      },
      undo: () => {
        this.m_value = backup;
        this.m_cache.pause = true;
      }
    });
  }

  add(delta: number) {
    if (equals(delta, 0)) return;

    HistoryManager.record({
      fn: () => {
        this.m_value += delta;
        this.m_cache.pause = true;
      },
      undo: () => {
        this.m_value -= delta;
        this.m_cache.pause = true;
      }
    });
  }

  tempAdd(value: number) {
    this.m_delta += value;
    this.m_cache.pause = true;
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
    this.m_cache.pause = true;
  }
}

export class Vec2Value implements TransformComponentValue<vec2> {
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

export class CachedVec2Value implements TransformComponentValue<vec2> {
  private m_value: vec2;
  private m_delta = vec2.create();
  private m_cache: Cache;

  constructor(cache: Cache, value: vec2 = [0, 0]) {
    this.m_value = vec2.clone(value);
    this.m_cache = cache;
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
        this.m_cache.pause = true;
      },
      undo: () => {
        vec2.copy(this.m_value, backup);
        this.m_cache.pause = true;
      }
    });
  }

  add(delta: vec2) {
    if (vec2.equals(delta, [0, 0])) return;

    HistoryManager.record({
      fn: () => {
        vec2.add(this.m_value, delta, this.m_value);
        this.m_cache.pause = true;
      },
      undo: () => {
        vec2.sub(this.m_value, delta, this.m_value);
        this.m_cache.pause = true;
      }
    });
  }

  tempAdd(value: vec2) {
    vec2.add(this.m_delta, value, this.m_delta);
    this.m_cache.pause = true;
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
    this.m_cache.pause = true;
  }
}

class Transform implements BaseTransformComponent {
  protected m_translation: TransformComponentValue<vec2>;
  protected m_rotation: TransformComponentValue<number>;

  constructor(position: vec2 = [0, 0], rotation: number = 0, cache?: Cache) {
    this.m_translation = cache ? new CachedVec2Value(cache, position) : new Vec2Value(position);
    this.m_rotation = cache ? new CachedFloatValue(cache, rotation) : new FloatValue(rotation);
  }

  get position(): vec2 {
    return this.m_translation.get();
  }

  set position(value: vec2) {
    this.m_translation.set(value);
  }

  get rotation(): number {
    return this.m_rotation.get();
  }

  set rotation(value: number) {
    this.m_rotation.set(value);
  }

  get staticPosition(): vec2 {
    return this.m_translation.getStatic();
  }

  get staticRotation(): number {
    return this.m_rotation.getStatic();
  }

  get positionDelta(): vec2 {
    return this.m_translation.getDelta();
  }

  get rotationDelta(): number {
    return this.m_rotation.getDelta();
  }

  tempTranslate(delta: vec2) {
    this.m_translation.tempAdd(delta);
  }

  tempRotate(delta: number) {
    this.m_rotation.tempAdd(delta);
  }

  translate(delta: vec2) {
    this.m_translation.add(delta);
  }

  rotate(delta: number) {
    this.m_rotation.add(delta);
  }

  apply() {
    this.m_translation.apply();
    this.m_rotation.apply();
  }

  clear() {
    this.m_translation.clear();
    this.m_rotation.clear();
  }
}

class UntrackedBaseTransform implements UntrackedBaseTransformComponent {
  protected m_translation: vec2;
  protected m_rotation: number;

  constructor(position: vec2 = [0, 0], rotation: number = 0) {
    this.m_translation = vec2.clone(position);
    this.m_rotation = rotation;
  }

  get position(): vec2 {
    return vec2.clone(this.m_translation);
  }

  set position(value: vec2) {
    vec2.copy(this.m_translation, value);
  }

  get rotation(): number {
    return this.m_rotation;
  }

  set rotation(value: number) {
    this.m_rotation = value;
  }

  get mat3(): mat3 {
    return mat3.fromTranslationRotation(
      this.m_translation,
      this.m_rotation,
      this.m_translation,
      this.m_translation
    );
  }

  translate(delta: vec2) {
    vec2.add(this.m_translation, delta, this.m_translation);
  }

  rotate(delta: number) {
    this.m_rotation += delta;
  }
}

export class RectTransform extends Transform implements RectTransformComponent {
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
    super(position, rotation);
    this.m_size = vec2.clone(size);
    this.m_reflection = new Vec2Value(reflect);
  }

  get size(): vec2 {
    const box = this.unrotatedBoundingBox;
    return vec2.sub(box[1], box[0]);
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

  get scaling(): vec2 {
    return vec2.clone(this.m_scale);
  }

  set scaling(value: vec2) {
    vec2.copy(this.m_scale, value);
  }

  get reflection(): vec2 {
    return vec2.sign(this.m_reflection.get());
  }

  set reflection(value: vec2) {
    this.m_reflection.set(vec2.sign(value, value));
  }

  get origin() {
    return vec2.clone(this.m_origin);
  }

  set origin(value: vec2) {
    vec2.rotate(value, this.center, -this.m_rotation.get(), this.m_origin);
  }

  get staticSize(): vec2 {
    return vec2.clone(this.m_size);
  }

  get center(): vec2 {
    return this.getCenter();
  }

  get staticBoundingBox(): Box {
    const position = this.m_translation.getStatic();
    return [position, vec2.add(position, this.m_size)];
  }

  get unrotatedBoundingBox(): Box {
    const position = this.m_translation.get();
    const box: Box = [position, vec2.add(position, this.m_size)];

    if (!vec2.exactEquals(this.m_scale, [1, 1])) {
      vec2.scale(box[0], this.m_origin, this.m_scale, box[0]);
      vec2.scale(box[1], this.m_origin, this.m_scale, box[1]);
    }

    return [vec2.min(box[0], box[1]), vec2.max(box[0], box[1])];
  }

  get rotatedBoundingBox(): [vec2, vec2, vec2, vec2] {
    const position = this.m_translation.getStatic();
    const unrotatedBox: Box = [position, vec2.add(position, this.m_size)];

    if (!vec2.exactEquals(this.m_scale, [1, 1])) {
      vec2.scale(unrotatedBox[0], this.m_origin, this.m_scale, unrotatedBox[0]);
      vec2.scale(unrotatedBox[1], this.m_origin, this.m_scale, unrotatedBox[1]);
    }

    const angle = this.m_rotation.get();
    const delta = vec2.rotate(this.m_translation.getDelta(), [0, 0], -angle);

    vec2.add(unrotatedBox[0], delta, unrotatedBox[0]);
    vec2.add(unrotatedBox[1], delta, unrotatedBox[1]);

    const box = [
      vec2.min(unrotatedBox[0], unrotatedBox[1]),
      vec2.max(unrotatedBox[0], unrotatedBox[1])
    ];

    const center = this.getCenter();

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

    this.rotatedBoundingBox.forEach((point) => {
      vec2.min(min, point, min);
      vec2.max(max, point, max);
    });

    return [min, max];
  }

  get mat3() {
    return mat3.fromTranslationRotationScaleReflection(
      this.m_translation.get(),
      this.m_rotation.get(),
      this.m_scale,
      this.m_reflection.get(),
      this.m_translation.getStatic(),
      this.getCenter(),
      this.m_origin
    );
  }

  private getCenter(position: vec2 = this.m_translation.getStatic(), size: vec2 = this.m_size) {
    const semiSize = vec2.divS(size, 2);
    return vec2.add(position, semiSize, semiSize);
  }

  transform(point: vec2) {
    if (this.m_rotation.get() === 0 && vec2.exactEquals(this.m_scale, [1, 1])) return point;
    return vec2.transformMat3(point, this.mat3);
  }

  tempScale(magnitude: vec2, normalizeRotation: boolean = false) {
    vec2.copy(this.m_scale, magnitude);
  }

  scale(magnitude: vec2, normalizeRotation: boolean = false) {
    this.tempScale(magnitude, normalizeRotation);
    this.apply();
  }

  apply() {
    super.apply();

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
    super.clear();
    vec2.set(this.m_scale, 1, 1);
  }

  asObject() {
    return {
      position: this.position,
      rotation: this.rotation,
      reflection: this.reflection
    };
  }
}

export class ElementTransform extends Transform implements ElementTransformComponent {
  private m_parent: ElementEntity;
  private m_cache: Cache;

  private m_scale = vec2.fromValues(1, 1);
  private m_origin = vec2.create();

  constructor(parent: ElementEntity, cache: Cache, position: vec2 = [0, 0], rotation: number = 0) {
    super(position, rotation, cache);
    this.m_parent = parent;
    this.m_cache = cache;
  }

  get origin() {
    return vec2.clone(this.m_origin);
  }

  set origin(value: vec2) {
    vec2.rotate(value, this.dynamicCenter, -this.m_rotation.get(), this.m_origin);
  }

  get dynamicCenter() {
    const box = this.unrotatedBoundingBox;
    const center = vec2.add(box[0], box[1]);
    vec2.divS(center, 2, center);

    return center;
  }

  get center() {
    const box = this.staticBoundingBox;
    const center = vec2.add(box[0], box[1]);
    vec2.divS(center, 2, center);

    return center;
  }

  private onStaticBoundingBoxCacheMiss(): Box {
    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    this.m_parent.forEachBezier((bezier) => {
      const box = bezier.boundingBox;
      vec2.min(min, box[0], min);
      vec2.max(max, box[1], max);
    });

    if (vec2.exactEquals(min, [Infinity, Infinity])) {
      this.m_parent.forEach((vertex) => {
        vec2.min(min, vertex.transform.staticPosition, min);
        vec2.max(max, vertex.transform.staticPosition, max);
      });
    }

    const position = this.m_translation.getStatic();

    return [vec2.add(min, position, min), vec2.add(max, position, max)];
  }

  get staticBoundingBox(): Box {
    Debugger.time('sBox');
    const box = this.m_cache.cached(
      'staticBoundingBox',
      this.onStaticBoundingBoxCacheMiss.bind(this)
    );
    Debugger.timeEnd('sBox');
    return box;
  }

  private onUnrotatedBoundingBoxCacheMiss(): Box {
    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    this.m_parent.forEachBezier((bezier) => {
      const box = bezier.boundingBox;
      vec2.min(min, box[0], min);
      vec2.max(max, box[1], max);
    });

    if (vec2.exactEquals(min, [Infinity, Infinity])) {
      this.m_parent.forEach((vertex) => {
        vec2.min(min, vertex.transform.position, min);
        vec2.max(max, vertex.transform.position, max);
      });
    }

    const position = this.m_translation.get();

    return [vec2.add(min, position, min), vec2.add(max, position, max)];
  }

  get unrotatedBoundingBox(): Box {
    Debugger.time('uBox');
    const box = this.m_cache.cached(
      'unrotatedBoundingBox',
      this.onUnrotatedBoundingBoxCacheMiss.bind(this)
    );

    Debugger.timeEnd('uBox');
    return box;
  }

  get rotatedBoundingBox(): [vec2, vec2, vec2, vec2] {
    Debugger.time('rBox');
    const box = this.unrotatedBoundingBox;
    const angle = this.m_rotation.get();

    const points: [vec2, vec2, vec2, vec2] = [
      vec2.clone(box[0]),
      [box[1][0], box[0][1]],
      vec2.clone(box[1]),
      [box[0][0], box[1][1]]
    ];

    if (angle === 0) return points;

    const center = this.dynamicCenter;
    Debugger.timeEnd('rBox');

    return [
      vec2.rotate(points[0], center, angle, points[0]),
      vec2.rotate(points[1], center, angle, points[1]),
      vec2.rotate(points[2], center, angle, points[2]),
      vec2.rotate(points[3], center, angle, points[3])
    ];
  }

  private onBoundingBoxCacheMiss(): Box {
    const angle = this.m_rotation.get();
    if (angle === 0) return this.unrotatedBoundingBox;

    const center = this.dynamicCenter;
    const position = this.m_translation.get();

    vec2.sub(center, position, center);

    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    this.m_parent.forEachBezier((bezier) => {
      const extrema = bezier.getRotatedExtrema(center, angle);
      extrema.forEach((point) => {
        vec2.min(min, point, min);
        vec2.max(max, point, max);
      });
    });

    if (vec2.exactEquals(min, [Infinity, Infinity])) {
      this.m_parent.forEach((vertex) => {
        vec2.min(min, vertex.transform.position, min);
        vec2.max(max, vertex.transform.position, max);
      });
    }

    return [vec2.add(min, position, min), vec2.add(max, position, max)];
  }

  get boundingBox(): Box {
    Debugger.time('bBox');
    const value = this.m_cache.cached('boundingBox', this.onBoundingBoxCacheMiss.bind(this));
    Debugger.timeEnd('bBox');
    return value;
  }

  private onLargeBoundingBoxCacheMiss(): Box {
    const box = this.boundingBox;
    const position = this.m_translation.get();

    let min: vec2 = vec2.sub(box[0], position);
    let max: vec2 = vec2.sub(box[1], position);

    this.m_parent.forEach((vertex) => {
      const vertexBox = vertex.boundingBox;

      vec2.min(min, vertexBox[0], min);
      vec2.max(max, vertexBox[1], max);
    });

    return [vec2.add(min, position, min), vec2.add(max, position, max)];
  }

  get largeBoundingBox(): Box {
    Debugger.time('lBox');
    const box = this.m_cache.cached(
      'largeBoundingBox',
      this.onLargeBoundingBoxCacheMiss.bind(this)
    );
    Debugger.timeEnd('lBox');

    return box;
  }

  private onMat3CacheMiss() {
    return mat3.fromTranslationRotation(
      this.m_translation.get(),
      this.m_rotation.get(),
      this.m_translation.getStatic(),
      this.center
    );
  }

  get mat3() {
    Debugger.time('mat3');
    const matrix = this.m_cache.cached('mat3', this.onMat3CacheMiss.bind(this));
    Debugger.timeEnd('mat3');

    return matrix;
  }

  transform(point: vec2): vec2 {
    const angle = this.m_rotation.get();
    if (angle === 0) return vec2.add(point, this.m_translation.get());

    const rotated = vec2.rotate(point, this.dynamicCenter, angle);
    vec2.add(rotated, this.m_translation.get(), rotated);

    return rotated;
  }

  tempScale(magnitude: vec2, normalizeRotation: boolean = false) {
    const v1 = this.transform([0, 0]);
    const origin = vec2.sub(this.m_origin, this.m_translation.get());
    const rotation = this.m_rotation.get();

    this.m_parent.forEach((vertex) => {
      const position = vertex.transform.staticPosition;
      const left = vertex.transform.staticLeft;
      const right = vertex.transform.staticRight;

      if (normalizeRotation) {
        vec2.rotate(position, origin, rotation, position);
        vec2.rotate(left, [0, 0], rotation, left);
        vec2.rotate(right, [0, 0], rotation, right);
      }

      vec2.scale(position, origin, magnitude, position);
      vec2.scale(left, [0, 0], magnitude, left);
      vec2.scale(right, [0, 0], magnitude, right);

      if (normalizeRotation) {
        vec2.rotate(position, origin, -rotation, position);
        vec2.rotate(left, [0, 0], -rotation, left);
        vec2.rotate(right, [0, 0], -rotation, right);
      }

      vertex.transform.tempPosition = position;
      vertex.transform.tempLeft = left;
      vertex.transform.tempRight = right;
    });

    const v2 = this.transform([0, 0]);

    this.m_translation.tempAdd(vec2.sub(v1, v2, v2));
    this.m_cache.pause = true;
  }

  scale(magnitude: vec2, normalizeRotation: boolean = false) {
    this.tempScale(magnitude, normalizeRotation);
    this.apply();
  }

  apply() {
    super.apply();
    this.m_parent.forEach((vertex) => {
      vertex.transform.apply();
    });

    vec2.set(this.m_scale, 1, 1);
  }

  clear() {
    super.clear();
    this.m_parent.forEach((vertex) => vertex.transform.clear());

    vec2.set(this.m_scale, 1, 1);
  }

  asObject() {
    return {
      position: this.position,
      rotation: this.rotation
    };
  }
}

export class SimpleTransform implements SimpleTransformComponent {
  private m_translation: TransformComponentValue<vec2>;

  constructor(position: vec2) {
    this.m_translation = new Vec2Value(position);
  }

  get position(): vec2 {
    return this.m_translation.get();
  }

  set position(value: vec2) {
    this.m_translation.set(value);
  }

  get staticPosition(): vec2 {
    return this.m_translation.getStatic();
  }

  get positionDelta(): vec2 {
    return this.m_translation.getDelta();
  }

  set positionDelta(value: vec2) {
    const current = this.m_translation.getDelta();
    this.m_translation.tempAdd(vec2.sub(value, current, current));
  }

  set tempPosition(value: vec2) {
    const current = this.m_translation.get();
    this.m_translation.tempAdd(vec2.sub(value, current, current));
  }

  get mat3(): mat3 {
    return mat3.fromTranslation(this.m_translation.get());
  }

  tempTranslate(delta: vec2) {
    this.m_translation.tempAdd(delta);
  }

  translate(delta: vec2) {
    this.m_translation.add(delta);
  }

  set cache(cache: Cache) {
    this.m_translation = new CachedVec2Value(cache, this.m_translation.get());
  }

  apply() {
    this.m_translation.apply();
  }

  clear() {
    this.m_translation.clear();
  }
}

export class VertexTransform implements VertexTransformComponent {
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

  get left(): vec2 {
    return this.m_left ? this.m_left.position : [0, 0];
  }

  set left(value: vec2) {
    if (this.m_left) {
      this.m_left.position = value;
    } else {
      this.m_vertex.left = new Handle({ position: value, type: 'bezier', parent: this.m_vertex });
    }
  }

  get right(): vec2 {
    return this.m_right ? this.m_right.position : [0, 0];
  }

  set right(value: vec2) {
    if (this.m_right) {
      this.m_right.position = value;
    } else {
      this.m_vertex.right = new Handle({ position: value, type: 'bezier', parent: this.m_vertex });
    }
  }

  get staticPosition(): vec2 {
    return this.m_position.staticPosition;
  }

  get staticLeft(): vec2 {
    return this.m_left ? this.m_left.staticPosition : [0, 0];
  }

  get staticRight(): vec2 {
    return this.m_right ? this.m_right.staticPosition : [0, 0];
  }

  get positionDelta(): vec2 {
    return this.m_position.positionDelta;
  }

  set positionDelta(value: vec2) {
    this.m_position.positionDelta = value;
  }

  get leftDelta(): vec2 {
    return this.m_left ? this.m_left.positionDelta : [0, 0];
  }

  set leftDelta(value: vec2) {
    if (this.m_left) this.m_left.positionDelta = value;
  }

  get rightDelta(): vec2 {
    return this.m_right ? this.m_right.positionDelta : [0, 0];
  }

  set rightDelta(value: vec2) {
    if (this.m_right) this.m_right.positionDelta = value;
  }

  set tempPosition(value: vec2) {
    this.m_position.tempPosition = value;
  }

  set tempLeft(value: vec2) {
    if (this.m_left) this.m_left.tempPosition = value;
  }

  set tempRight(value: vec2) {
    if (this.m_right) this.m_right.tempPosition = value;
  }

  get mat3(): mat3 {
    return mat3.fromTranslation(this.m_position.position);
  }

  tempTranslate(delta: vec2): void {
    this.m_position.tempTranslate(delta);
  }

  tempTranslateLeft(delta: vec2, lockMirror?: boolean | undefined): void {
    this.m_left?.tempTranslate(delta);

    if (!lockMirror && this.m_right) {
      const direction = vec2.unit(vec2.neg(this.left));
      if (!vec2.equals(direction, [0, 0])) {
        this.tempTranslateRight(
          vec2.sub(vec2.mulS(direction, vec2.len(this.right!)), this.right),
          true
        );
      }
    }
  }

  tempTranslateRight(delta: vec2, lockMirror?: boolean | undefined): void {
    this.m_right?.tempTranslate(delta);

    if (!lockMirror && this.m_left) {
      const direction = vec2.unit(vec2.neg(this.right));
      if (!vec2.equals(direction, [0, 0])) {
        this.tempTranslateLeft(
          vec2.sub(vec2.mulS(direction, vec2.len(this.left!)), this.left),
          true
        );
      }
    }
  }

  translate(delta: vec2): void {
    this.m_position.translate(delta);
  }

  translateLeft(delta: vec2): void {
    this.m_left?.translate(delta);
  }

  translateRight(delta: vec2): void {
    this.m_right?.translate(delta);
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

export class UntrackedTransform
  extends UntrackedBaseTransform
  implements UntrackedTransformComponent
{
  private m_center = vec2.create();

  constructor(position?: vec2, rotation?: number) {
    super(position, rotation);
  }

  get center(): vec2 {
    return vec2.clone(this.m_center);
  }

  set center(value: vec2) {
    vec2.copy(this.m_center, value);
  }

  get staticBoundingBox(): Box {
    return [this.m_translation, this.m_translation];
  }

  get unrotatedBoundingBox(): Box {
    return [this.m_translation, this.m_translation];
  }

  get rotatedBoundingBox(): [vec2, vec2, vec2, vec2] {
    return [this.m_translation, this.m_translation, this.m_translation, this.m_translation];
  }

  get boundingBox(): Box {
    return [this.m_translation, this.m_translation];
  }

  get mat3(): mat3 {
    return mat3.fromTranslationRotation(
      this.m_translation,
      this.m_rotation,
      this.m_translation,
      this.m_center
    );
  }

  transform(point: vec2): vec2 {
    if (this.m_rotation === 0) return vec2.add(point, this.m_translation);

    const rotated = vec2.rotate(point, this.center, this.m_rotation);
    vec2.add(rotated, this.m_translation, rotated);

    return rotated;
  }
}

export class UntrackedSimpleTransform implements UntrackedSimpleTransformComponent {
  private m_translation: vec2;

  constructor(position: vec2 = [0, 0]) {
    this.m_translation = vec2.clone(position);
  }

  get position(): vec2 {
    return vec2.clone(this.m_translation);
  }

  set position(value: vec2) {
    vec2.copy(this.m_translation, value);
  }

  get mat3(): mat3 {
    return mat3.fromTranslation(this.m_translation);
  }

  translate(delta: vec2) {
    vec2.add(this.m_translation, delta, this.m_translation);
  }
}
