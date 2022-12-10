import CommandHistory from '@/editor/history/history';
import { mat3, vec2 } from '@/math';
import { Cache } from '@/editor/ecs/components/cache';
import { FloatValue, Vec2Value } from '@/editor/history/value';
import {
  ChangeCommand,
  ChangePrimitiveCommand,
  ChangeVec2Command,
  FunctionCallCommand,
  PauseCacheCommand
} from '@/editor/history/command';
import Handle from '../entities/handle';

export const isCompleteTransform = (b: GenericTransformComponent): b is TransformComponent => {
  return b instanceof Transform || b instanceof UntrackedTransform;
};

class TransformVec2Value extends Vec2Value implements TransformComponentValue<vec2> {
  protected m_static: vec2;

  constructor(value?: vec2) {
    super(value);
    this.m_static = vec2.clone(this.m_value);
  }

  get static(): vec2 {
    return vec2.clone(this.m_static);
  }

  get delta(): vec2 {
    return vec2.sub(this.m_value, this.m_static);
  }

  set delta(amount: vec2) {
    super.add(vec2.sub(amount, this.delta));
  }

  apply(): void {
    if (vec2.exactEquals(this.m_static, this.m_value)) return;
    CommandHistory.add(new ChangeVec2Command(this.m_static, this.m_value));
  }
}

class TransformFloatValue extends FloatValue implements TransformComponentValue<number> {
  protected m_static: { value: number };

  constructor(value?: number) {
    super(value);
    this.m_static = { value: value || 0 };
  }

  get static(): number {
    return this.m_static.value;
  }

  get delta(): number {
    return this.m_value.value - this.m_static.value;
  }

  set delta(amount: number) {
    super.add(amount - this.delta);
  }

  apply(): void {
    if (this.m_static.value === this.m_value.value) return;
    CommandHistory.add(new ChangePrimitiveCommand(this.m_static, this.m_value.value));
  }
}

class CachedTransformVec2Value extends TransformVec2Value {
  private m_cache: CacheComponent;

  constructor(cache: CacheComponent, value?: vec2) {
    super(value);
    this.m_cache = cache;
  }

  get cache(): CacheComponent {
    return this.m_cache;
  }

  set cache(cache: CacheComponent) {
    this.m_cache = cache;
  }

  get value(): vec2 {
    return vec2.clone(this.m_value);
  }

  set value(value: vec2) {
    super.value = value;
    this.m_cache.pause = true;
  }

  get delta(): vec2 {
    return vec2.sub(this.m_value, this.m_static);
  }

  set delta(amount: vec2) {
    this.add(vec2.sub(amount, this.delta));
  }

  add(amount: vec2): void {
    if (amount[0] === 0 && amount[1] === 0) return;
    super.add(amount);

    this.m_cache.pause = true;
  }

  apply(): void {
    if (vec2.exactEquals(this.m_static, this.m_value)) return;
    super.apply();

    CommandHistory.add(new PauseCacheCommand(this.m_cache));
  }
}

class CachedTransformFloatValue extends TransformFloatValue {
  private m_cache: CacheComponent;

  constructor(cache: CacheComponent, value?: number) {
    super(value);
    this.m_cache = cache;
  }

  get cache(): CacheComponent {
    return this.m_cache;
  }

  set cache(cache: CacheComponent) {
    this.m_cache = cache;
  }

  get value(): number {
    return this.m_value.value;
  }

  set value(value: number) {
    super.value = value;
    this.m_cache.pause = true;
  }

  get delta(): number {
    return this.m_value.value - this.m_static.value;
  }

  set delta(amount: number) {
    this.add(amount - this.delta);
  }

  add(amount: number): void {
    if (amount === 0) return;
    super.add(amount);

    this.m_cache.pause = true;
  }

  apply(): void {
    if (this.m_static.value === this.m_value.value) return;
    super.apply();

    CommandHistory.add(new PauseCacheCommand(this.m_cache));
  }
}

// TODO: fix
class UntrackedTransformVec2Value implements TransformComponentValue<vec2> {
  private m_value: vec2;

  constructor(value?: vec2) {
    this.m_value = value ? vec2.clone(value) : vec2.create();
  }

  get value(): vec2 {
    return vec2.clone(this.m_value);
  }

  set value(value: vec2) {
    vec2.copy(this.m_value, value);
  }

  get static(): vec2 {
    return vec2.clone(this.m_value);
  }

  get delta(): vec2 {
    return [0, 0];
  }

  set delta(amount: vec2) {
    this.add(amount);
  }

  add(amount: vec2): void {
    if (amount[0] === 0 && amount[1] === 0) return;
    vec2.add(this.m_value, amount);
  }

  animateTo(value: vec2 | null): void {}

  apply(): void {}
}

class UntrackedTransformFloatValue implements TransformComponentValue<number> {
  private m_value: number;

  constructor(value?: number) {
    this.m_value = value || 0;
  }

  get value(): number {
    return this.m_value;
  }

  set value(value: number) {
    this.m_value = value;
  }

  get static(): number {
    return this.m_value;
  }

  get delta(): number {
    return 0;
  }

  set delta(amount: number) {
    this.add(amount);
  }

  add(amount: number): void {
    this.m_value += amount;
  }

  animateTo(value: number | null): void {}

  apply(): void {}
}

export class SimpleTransform implements SimpleTransformComponent {
  position: TransformComponentValue<vec2>;

  constructor(position: vec2 = [0, 0]) {
    this.position = new TransformVec2Value(position);
  }

  set cache(cache: CacheComponent) {
    if (this.position instanceof CachedTransformVec2Value) {
      const backup = this.position.cache;
      if (backup === cache) return;

      CommandHistory.add(
        new ChangeCommand(
          () => {
            (<CachedTransformVec2Value>this.position).cache = cache;
          },
          () => {
            (<CachedTransformVec2Value>this.position).cache = backup;
          }
        )
      );

      return;
    }

    const backup = this.position;
    const value = new CachedTransformVec2Value(cache, this.position.value);

    CommandHistory.add(
      new ChangeCommand(
        () => {
          this.position = value;
        },
        () => {
          this.position = backup;
        }
      )
    );
  }

  get boundingBox(): Box {
    return [this.position.value, this.position.value];
  }

  get mat3(): mat3 {
    return mat3.fromTranslation(this.position.value);
  }

  translate(amount: vec2, apply: boolean = false): void {
    this.position.add(amount);
    if (apply) this.position.apply();
  }

  transform(point: vec2): vec2 {
    return vec2.add(point, this.position.value);
  }

  apply(): void {
    this.position.apply();
  }

  asObject(): TransformComponentObject {
    return {
      position: this.position.value
    };
  }
}

export class Transform implements TransformComponent {
  readonly position: TransformComponentValue<vec2>;
  readonly rotation: TransformComponentValue<number>;

  protected m_origin = vec2.create();

  constructor(position: vec2 = [0, 0], rotation: number = 0, cache?: CacheComponent) {
    this.position = cache
      ? new CachedTransformVec2Value(cache, position)
      : new TransformVec2Value(position);
    this.rotation = cache
      ? new CachedTransformFloatValue(cache, rotation)
      : new TransformFloatValue(rotation);
  }

  get origin(): vec2 {
    return vec2.clone(this.m_origin);
  }

  set origin(value: vec2) {
    vec2.copy(this.m_origin, value);
  }

  get size(): vec2 {
    return [0, 0];
  }

  get center(): vec2 {
    return [0, 0];
  }

  get staticCenter(): vec2 {
    return [0, 0];
  }

  get staticBoundingBox(): Box {
    return [
      [0, 0],
      [0, 0]
    ];
  }

  get unrotatedBoundingBox(): Box {
    return [
      [0, 0],
      [0, 0]
    ];
  }

  get rotatedBoundingBox(): [vec2, vec2, vec2, vec2] {
    return [
      [0, 0],
      [0, 0],
      [0, 0],
      [0, 0]
    ];
  }

  get boundingBox(): Box {
    return [
      [0, 0],
      [0, 0]
    ];
  }

  get mat3(): mat3 {
    return mat3.create();
  }

  translate(amount: vec2, apply: boolean = false): void {
    this.position.add(amount);
    if (apply) this.position.apply();
  }

  rotate(amount: number, apply: boolean = false): void {
    this.rotation.add(amount);
    if (apply) this.rotation.apply();
  }

  scale(amount: vec2, normalizeRotation?: boolean | undefined, apply?: boolean | undefined): void {}

  transform(point: vec2): vec2 {
    const angle = this.rotation.value;
    if (angle === 0) return vec2.add(point, this.position.value);

    const translated = vec2.add(point, this.position.value);
    return vec2.rotate(translated, this.center, angle, translated);
  }

  apply(): void {
    this.position.apply();
    this.rotation.apply();
  }

  asObject(): TransformComponentObject {
    return {
      position: this.position.value,
      rotation: this.rotation.value
    };
  }
}

export class ElementTransform extends Transform implements ElementTransformComponent {
  private m_parent: ElementEntity;
  private m_cache: CacheComponent;

  constructor(parent: ElementEntity, cache: Cache, position?: vec2, rotation?: number) {
    super(position, rotation, cache);
    this.m_parent = parent;
    this.m_cache = cache;
  }

  set origin(value: vec2) {
    vec2.rotate(value, this.center, -this.rotation.value, this.m_origin);
  }

  private onSizeCacheMiss(): vec2 {
    const box = this.unrotatedBoundingBox;
    return vec2.sub(box[1], box[0]);
  }

  get size(): vec2 {
    return this.m_cache.cached('size', this.onSizeCacheMiss.bind(this));
  }

  private onCenterCacheMiss(): vec2 {
    const box = this.unrotatedBoundingBox;
    return vec2.mid(box[0], box[1]);
  }

  get center() {
    return this.m_cache.cached('center', this.onCenterCacheMiss.bind(this));
  }

  private onStaticCenterCacheMiss(): vec2 {
    const box = this.staticBoundingBox;
    return vec2.mid(box[0], box[1]);
  }

  get staticCenter() {
    return this.m_cache.cached('staticCenter', this.onStaticCenterCacheMiss.bind(this));
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
        vec2.min(min, vertex.transform.position.static, min);
        vec2.max(max, vertex.transform.position.static, max);
      });
    }

    const position = this.position.static;

    return [vec2.add(min, position, min), vec2.add(max, position, max)];
  }

  get staticBoundingBox(): Box {
    return this.m_cache.cached('staticBoundingBox', this.onStaticBoundingBoxCacheMiss.bind(this));
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
        vec2.min(min, vertex.transform.position.value, min);
        vec2.max(max, vertex.transform.position.value, max);
      });
    }

    const position = this.position.value;

    return [vec2.add(min, position, min), vec2.add(max, position, max)];
  }

  get unrotatedBoundingBox(): Box {
    return this.m_cache.cached(
      'unrotatedBoundingBox',
      this.onUnrotatedBoundingBoxCacheMiss.bind(this)
    );
  }

  private onRotatedBoundingBoxCacheMiss(): [vec2, vec2, vec2, vec2] {
    const box = this.unrotatedBoundingBox;
    const angle = this.rotation.value;

    const points: [vec2, vec2, vec2, vec2] = [
      vec2.clone(box[0]),
      [box[1][0], box[0][1]],
      vec2.clone(box[1]),
      [box[0][0], box[1][1]]
    ];

    if (angle === 0) return points;

    const center = this.center;

    return [
      vec2.rotate(points[0], center, angle, points[0]),
      vec2.rotate(points[1], center, angle, points[1]),
      vec2.rotate(points[2], center, angle, points[2]),
      vec2.rotate(points[3], center, angle, points[3])
    ];
  }

  get rotatedBoundingBox(): [vec2, vec2, vec2, vec2] {
    return this.m_cache.cached('rotatedBoundingBox', this.onRotatedBoundingBoxCacheMiss.bind(this));
  }

  private onBoundingBoxCacheMiss(): Box {
    const angle = this.rotation.value;
    if (angle === 0) return this.unrotatedBoundingBox;

    const center = this.center;
    const position = this.position.value;
    const translatedCenter = vec2.sub(center, position);

    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    this.m_parent.forEachBezier((bezier) => {
      const extrema = bezier.getRotatedExtrema(translatedCenter, angle);
      extrema.forEach((point) => {
        vec2.min(min, point, min);
        vec2.max(max, point, max);
      });
    });

    if (vec2.exactEquals(min, [Infinity, Infinity])) {
      this.m_parent.forEach((vertex) => {
        vec2.min(min, vertex.transform.position.value, min);
        vec2.max(max, vertex.transform.position.value, max);
      });
    }

    return [vec2.add(min, position, min), vec2.add(max, position, max)];
  }

  get boundingBox(): Box {
    return this.m_cache.cached('boundingBox', this.onBoundingBoxCacheMiss.bind(this));
  }

  private onLargeBoundingBoxCacheMiss(): Box {
    const box = this.boundingBox;
    const position = this.position.value;

    let min: vec2 = vec2.sub(box[0], position);
    let max: vec2 = vec2.sub(box[1], position);

    this.m_parent.forEach((vertex) => {
      const vertexBox = vertex.transform.boundingBox;

      vec2.min(min, vertexBox[0], min);
      vec2.max(max, vertexBox[1], max);
    });

    return [vec2.add(min, position, min), vec2.add(max, position, max)];
  }

  get largeBoundingBox(): Box {
    return this.m_cache.cached('largeBoundingBox', this.onLargeBoundingBoxCacheMiss.bind(this));
  }

  private onMat3CacheMiss() {
    return mat3.fromTranslationRotation(
      this.position.value,
      this.rotation.value,
      this.position.static,
      this.staticCenter
    );
  }

  get mat3() {
    return this.m_cache.cached('mat3', this.onMat3CacheMiss.bind(this));
  }

  private applyScale() {
    this.m_parent.forEach((vertex) => {
      vertex.transform.apply();
    });
  }

  scale(amount: vec2, normalizeRotation: boolean = false, apply: boolean = true): void {
    const center = this.center;
    const origin = vec2.sub(this.m_origin, this.position.value);
    const rotation = this.rotation.value;

    this.m_parent.forEach((vertex) => {
      const position = vertex.transform.position.static;
      const left = vertex.transform.left?.static || [0, 0];
      const right = vertex.transform.right?.static || [0, 0];

      if (normalizeRotation) {
        vec2.rotate(position, origin, rotation, position);
        vec2.rotate(left, [0, 0], rotation, left);
        vec2.rotate(right, [0, 0], rotation, right);
      }

      vec2.scale(position, origin, amount, position);
      vec2.scale(left, [0, 0], amount, left);
      vec2.scale(right, [0, 0], amount, right);

      if (normalizeRotation) {
        vec2.rotate(position, origin, -rotation, position);
        vec2.rotate(left, [0, 0], -rotation, left);
        vec2.rotate(right, [0, 0], -rotation, right);
      }

      vertex.transform.position.value = position;
      if (vertex.transform.left) vertex.transform.left.value = left;
      if (vertex.transform.right) vertex.transform.right.value = right;

      vertex.pauseCache();
    });

    this.keepCentered(center, false);

    if (apply) this.applyScale();
  }

  untransform(point: vec2): vec2 {
    const angle = this.rotation.value;
    const untransformed = angle === 0 ? vec2.clone(point) : vec2.rotate(point, this.center, -angle);

    return vec2.sub(untransformed, this.position.value, untransformed);
  }

  keepCentered(center: vec2, apply: boolean = false): void {
    const angle = this.rotation.value;
    if (angle === 0) return;

    const updated = this.center;
    const sin = Math.sin(angle),
      cos = Math.cos(angle);

    this.position.add([
      center[0] -
        center[0] * cos +
        center[1] * sin -
        (updated[0] - updated[0] * cos + updated[1] * sin),
      center[1] -
        center[0] * sin -
        center[1] * cos -
        (updated[1] - updated[0] * sin - updated[1] * cos)
    ]);

    if (apply) this.position.apply();
  }

  apply() {
    super.apply();
    this.applyScale();
  }
}

export class FreehandTransform extends Transform implements FreehandTransformComponent {
  private m_parent: FreehandEntity;
  private m_cache: CacheComponent;

  constructor(parent: FreehandEntity, cache: CacheComponent, position?: vec2, rotation?: number) {
    super(position, rotation);
    this.m_parent = parent;
    this.m_cache = cache;
  }

  set origin(value: vec2) {
    vec2.rotate(value, this.center, -this.rotation.value, this.m_origin);
  }

  private onSizeCacheMiss(): vec2 {
    const box = this.unrotatedBoundingBox;
    return vec2.sub(box[1], box[0]);
  }

  get size(): vec2 {
    return this.m_cache.cached('size', this.onSizeCacheMiss.bind(this));
  }

  private onCenterCacheMiss(): vec2 {
    const box = this.unrotatedBoundingBox;
    return vec2.mid(box[0], box[1]);
  }

  get center() {
    return this.m_cache.cached('center', this.onCenterCacheMiss.bind(this));
  }

  private onStaticCenterCacheMiss(): vec2 {
    const box = this.staticBoundingBox;
    return vec2.mid(box[0], box[1]);
  }

  get staticCenter() {
    return this.m_cache.cached('staticCenter', this.onStaticCenterCacheMiss.bind(this));
  }

  private onStaticBoundingBoxCacheMiss(): Box {
    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    this.m_parent.forEach((point) => {
      const position = point.position.value;
      vec2.min(min, position, min);
      vec2.max(max, position, max);
    });

    const position = this.position.static;

    return [vec2.add(min, position, min), vec2.add(max, position, max)];
  }

  get staticBoundingBox(): Box {
    return this.m_cache.cached('staticBoundingBox', this.onStaticBoundingBoxCacheMiss.bind(this));
  }

  private onUnrotatedBoundingBoxCacheMiss(): Box {
    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    this.m_parent.forEach((point) => {
      const position = point.position.value;
      vec2.min(min, position, min);
      vec2.max(max, position, max);
    });

    const position = this.position.value;

    return [vec2.add(min, position, min), vec2.add(max, position, max)];
  }

  get unrotatedBoundingBox(): Box {
    return this.m_cache.cached(
      'unrotatedBoundingBox',
      this.onUnrotatedBoundingBoxCacheMiss.bind(this)
    );
  }

  private onRotatedBoundingBoxCacheMiss(): [vec2, vec2, vec2, vec2] {
    const box = this.unrotatedBoundingBox;
    const angle = this.rotation.value;

    const points: [vec2, vec2, vec2, vec2] = [
      vec2.clone(box[0]),
      [box[1][0], box[0][1]],
      vec2.clone(box[1]),
      [box[0][0], box[1][1]]
    ];

    if (angle === 0) return points;

    const center = this.center;

    return [
      vec2.rotate(points[0], center, angle, points[0]),
      vec2.rotate(points[1], center, angle, points[1]),
      vec2.rotate(points[2], center, angle, points[2]),
      vec2.rotate(points[3], center, angle, points[3])
    ];
  }

  // TODO: Move common methods in Transform class
  get rotatedBoundingBox(): [vec2, vec2, vec2, vec2] {
    return this.m_cache.cached('rotatedBoundingBox', this.onRotatedBoundingBoxCacheMiss.bind(this));
  }

  private onBoundingBoxCacheMiss(): Box {
    const angle = this.rotation.value;
    if (angle === 0) return this.unrotatedBoundingBox;

    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    const box = this.rotatedBoundingBox;
    box.forEach((point) => {
      vec2.min(point, min, min);
      vec2.max(point, max, max);
    });

    return [min, max];
  }

  get boundingBox(): Box {
    return this.m_cache.cached('boundingBox', this.onBoundingBoxCacheMiss.bind(this));
  }

  private onMat3CacheMiss() {
    return mat3.fromTranslationRotation(
      this.position.value,
      this.rotation.value,
      this.position.static,
      this.staticCenter
    );
  }

  get mat3(): mat3 {
    return this.m_cache.cached('mat3', this.onMat3CacheMiss.bind(this));
  }

  private applyScale() {
    this.m_parent.forEach((point) => {
      point.apply();
    });
  }

  scale(amount: vec2, normalizeRotation: boolean = false, apply: boolean = true): void {
    const center = this.center;
    const origin = vec2.sub(this.m_origin, this.position.value);
    const rotation = this.rotation.value;

    this.m_parent.forEach((point) => {
      const position = point.position.static;

      if (normalizeRotation) vec2.rotate(position, origin, rotation, position);

      vec2.scale(position, origin, amount, position);

      if (normalizeRotation) vec2.rotate(position, origin, -rotation, position);

      point.position.value = position;
    });

    this.keepCentered(center, false);
    CommandHistory.add(new PauseCacheCommand(this.m_cache));

    if (apply) this.applyScale();
  }

  keepCentered(center: vec2, apply: boolean = false): void {
    const angle = this.rotation.value;
    if (angle === 0) return;

    const updated = this.center;
    const sin = Math.sin(angle),
      cos = Math.cos(angle);

    this.position.add([
      center[0] -
        center[0] * cos +
        center[1] * sin -
        (updated[0] - updated[0] * cos + updated[1] * sin),
      center[1] -
        center[0] * sin -
        center[1] * cos -
        (updated[1] - updated[0] * sin - updated[1] * cos)
    ]);

    if (apply) this.position.apply();
  }

  apply() {
    super.apply();
    this.applyScale();
  }

  translate(amount: vec2, apply?: boolean): void {
    super.translate(amount);
    this.m_cache.pause;
    if (apply) this.position.apply();
  }
}

export class RectTransform extends Transform implements RectTransformComponent {
  readonly reflection: Vec2Value;

  private m_scale = vec2.fromValues(1, 1);
  private m_size: Vec2Value;

  constructor(
    position: vec2 = [0, 0],
    rotation: number = 0,
    size: vec2 = [0, 0],
    reflect: vec2 = [1, 1]
  ) {
    super(position, rotation);

    this.m_size = new Vec2Value(size);
    this.reflection = new Vec2Value(reflect);
  }

  get size(): vec2 {
    const box = this.unrotatedBoundingBox;
    return vec2.sub(box[1], box[0]);
  }

  set size(value: vec2) {
    this.m_size.value = value;
  }

  get scaling(): vec2 {
    return vec2.clone(this.m_scale);
  }

  set scaling(value: vec2) {
    vec2.copy(this.m_scale, value);
  }

  set origin(value: vec2) {
    vec2.rotate(value, this.staticCenter, -this.rotation.value, this.m_origin);
  }

  get staticSize(): vec2 {
    return this.m_size.value;
  }

  get staticCenter(): vec2 {
    return this.getCenter();
  }

  get staticBoundingBox(): Box {
    const position = this.position.static;
    return [position, vec2.add(position, this.m_size.value)];
  }

  get unrotatedBoundingBox(): Box {
    const position = this.position.value;
    const box: Box = [position, vec2.add(position, this.m_size.value)];

    if (!vec2.exactEquals(this.m_scale, [1, 1])) {
      vec2.scale(box[0], this.m_origin, this.m_scale, box[0]);
      vec2.scale(box[1], this.m_origin, this.m_scale, box[1]);
    }

    return [vec2.min(box[0], box[1]), vec2.max(box[0], box[1])];
  }

  get rotatedBoundingBox(): [vec2, vec2, vec2, vec2] {
    const position = this.position.static;
    const unrotatedBox: Box = [position, vec2.add(position, this.m_size.value)];

    if (!vec2.exactEquals(this.m_scale, [1, 1])) {
      vec2.scale(unrotatedBox[0], this.m_origin, this.m_scale, unrotatedBox[0]);
      vec2.scale(unrotatedBox[1], this.m_origin, this.m_scale, unrotatedBox[1]);
    }

    const angle = this.rotation.value;
    const delta = vec2.rotate(this.position.delta, [0, 0], -angle);

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
    if (this.rotation.value === 0) return this.unrotatedBoundingBox;

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
      this.position.value,
      this.rotation.value,
      this.m_scale,
      this.reflection.value,
      this.position.static,
      this.getCenter(),
      this.m_origin
    );
  }

  private getCenter(position: vec2 = this.position.static, size: vec2 = this.m_size.value) {
    const semiSize = vec2.divS(size, 2);
    return vec2.add(position, semiSize, semiSize);
  }

  scale(amount: vec2, normalizeRotation: boolean = true, apply: boolean = false): void {
    vec2.copy(this.m_scale, amount);

    if (apply) this.apply();
  }

  transform(point: vec2): vec2 {
    if (this.rotation.value === 0 && vec2.exactEquals(this.m_scale, [1, 1])) return point;
    return vec2.transformMat3(point, this.mat3);
  }

  apply(): void {
    const box = this.unrotatedBoundingBox;
    const size = vec2.sub(box[1], box[0]);

    if (!vec2.equals(this.m_size.value, size)) {
      const rotation = this.rotation.value;
      const position = vec2.rotate([0, 0], this.getCenter(), rotation);
      const reflection = this.reflection.value;

      vec2.sub(position, vec2.rotate([0, 0], this.getCenter(box[0], size), rotation), position);
      vec2.add(position, box[0], position);

      if (this.m_scale[0] < 0) reflection[0] = reflection[0] === 1 ? -1 : 1;
      if (this.m_scale[1] < 0) reflection[1] = reflection[1] === 1 ? -1 : 1;

      this.reflection.value = reflection;
      this.position.value = position;
      this.m_size.value = size;
      vec2.set(this.m_scale, 1, 1);
    }

    super.apply();
  }

  asObject(): TransformComponentObject {
    return {
      ...super.asObject(),
      reflection: this.reflection.value
    };
  }
}

export class VertexTransform implements VertexTransformComponent {
  private m_parent: VertexEntity;

  constructor(parent: VertexEntity) {
    this.m_parent = parent;
  }

  get position(): TransformComponentValue<vec2> {
    return this.m_parent.position.transform.position;
  }

  get left(): TransformComponentValue<vec2> | undefined {
    return this.m_parent.left?.transform.position;
  }

  set leftValue(value: vec2) {
    if (this.m_parent.left) {
      this.m_parent.left.transform.position.value = value;
    } else {
      const handle = new Handle({ position: value, type: 'bezier', parent: this.m_parent });
      this.m_parent.left = handle;
    }
  }

  get right(): TransformComponentValue<vec2> | undefined {
    return this.m_parent.right?.transform.position;
  }

  set rightValue(value: vec2) {
    if (this.m_parent.right) {
      this.m_parent.right.transform.position.value = value;
    } else {
      this.m_parent.right = new Handle({ position: value, type: 'bezier', parent: this.m_parent });
    }
  }

  get boundingBox(): Box {
    let min: vec2 = [0, 0];
    let max: vec2 = [0, 0];

    const position = this.m_parent.position.transform.position.value;
    const left = this.m_parent.left?.transform.position.value;
    const right = this.m_parent.right?.transform.position.value;

    if (left) {
      vec2.min(min, left, min);
      vec2.max(max, left, max);
    }

    if (right) {
      vec2.min(min, right, min);
      vec2.max(max, right, max);
    }

    return [vec2.add(min, position, min), vec2.add(max, position, max)];
  }

  translate(amount: vec2, apply?: boolean | undefined): void {
    this.m_parent.position.transform.position.add(amount);
    if (apply) this.m_parent.position.transform.position.apply();
  }

  translateLeft(amount: vec2, lockMirror?: boolean | undefined, apply?: boolean | undefined): void {
    if (!this.m_parent.left) return;

    this.m_parent.left.transform.position.add(amount);

    if (!lockMirror && this.m_parent.right) {
      const direction = vec2.normalize(vec2.neg(this.m_parent.left.transform.position.value));

      if (!vec2.equals(direction, [0, 0])) {
        const right = this.m_parent.right.transform.position.value;
        this.translateRight(vec2.sub(vec2.mulS(direction, vec2.len(right)), right), true, apply);
      }
    }

    if (apply) this.m_parent.left.transform.apply();
  }

  translateRight(
    amount: vec2,
    lockMirror?: boolean | undefined,
    apply?: boolean | undefined
  ): void {
    if (!this.m_parent.right) return;

    this.m_parent.right.transform.position.add(amount);

    if (!lockMirror && this.m_parent.left) {
      const direction = vec2.normalize(vec2.neg(this.m_parent.right.transform.position.value));

      if (!vec2.equals(direction, [0, 0])) {
        const left = this.m_parent.left.transform.position.value;
        this.translateLeft(vec2.sub(vec2.mulS(direction, vec2.len(left)), left), true, apply);
      }
    }

    if (apply) this.m_parent.right.transform.apply();
  }

  transform(point: vec2) {
    return vec2.add(point, this.position.value);
  }

  apply(): void {
    this.m_parent.position.transform.apply();
    this.m_parent.left?.transform.apply();
    this.m_parent.right?.transform.apply();
  }
}

export class UntrackedTransform implements TransformComponent {
  readonly position: TransformComponentValue<vec2>;
  readonly rotation: TransformComponentValue<number>;

  private m_size: vec2;

  origin: vec2;

  constructor(position: vec2 = [0, 0], rotation: number = 0, size: vec2 = [0, 0]) {
    this.position = new UntrackedTransformVec2Value(position);
    this.rotation = new UntrackedTransformFloatValue(rotation);
    this.m_size = vec2.clone(size);
  }

  get size(): vec2 {
    return vec2.clone(this.m_size);
  }

  set size(value: vec2) {
    vec2.copy(this.m_size, value);
  }

  get center(): vec2 {
    const box = this.unrotatedBoundingBox;
    return vec2.mid(box[0], box[1]);
  }

  get staticCenter(): vec2 {
    return this.center;
  }

  get staticBoundingBox(): Box {
    return this.unrotatedBoundingBox;
  }

  get unrotatedBoundingBox(): Box {
    const p1 = this.position.value,
      p2 = vec2.add(p1, this.m_size);
    return [vec2.min(p1, p2), vec2.max(p1, p2)];
  }

  get rotatedBoundingBox(): [vec2, vec2, vec2, vec2] {
    const box = this.unrotatedBoundingBox;
    const center = this.center;

    return [
      vec2.rotate(box[0], center, this.rotation.value),
      vec2.rotate([box[1][0], box[0][1]], center, this.rotation.value),
      vec2.rotate(box[1], center, this.rotation.value),
      vec2.rotate([box[0][0], box[1][1]], center, this.rotation.value)
    ];
  }

  get boundingBox(): Box {
    if (this.rotation.value === 0) return this.unrotatedBoundingBox;

    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    this.rotatedBoundingBox.forEach((point) => {
      vec2.min(min, point, min);
      vec2.max(max, point, max);
    });

    return [min, max];
  }

  get mat3(): mat3 {
    return mat3.fromTranslationRotation(
      this.position.value,
      this.rotation.value,
      this.position.value,
      this.center
    );
  }

  translate(amount: vec2): void {
    this.position.add(amount);
  }

  rotate(amount: number): void {
    this.rotation.add(amount);
  }

  scale(amount: vec2, normalizeRotation?: boolean | undefined, apply?: boolean | undefined): void {}

  transform(point: vec2): vec2 {
    const angle = this.rotation.value;
    if (angle === 0) return vec2.add(point, this.position.value);

    const rotated = vec2.rotate(point, this.center, angle);
    vec2.add(rotated, this.position.value, rotated);

    return rotated;
  }

  apply(): void {}

  asObject(): TransformComponentObject {
    return {
      position: this.position.value,
      rotation: this.rotation.value
    };
  }
}
