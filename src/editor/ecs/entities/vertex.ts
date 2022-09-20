import { isPointInBox, vec2, vec4 } from '@math';
import { nanoid } from 'nanoid';
import HistoryManager from '../../history';
import { Renderer } from '../../renderer';
import SceneManager from '../../scene';
import Element from './element';
import Handle from './handle';

class Vertex implements VertexEntity {
  readonly id: string;
  readonly type: EntityType = 'vertex';

  parent: ElementEntity;

  private m_position: HandleEntity;
  private m_left?: HandleEntity;
  private m_right?: HandleEntity;

  constructor({ id = nanoid(), position, left, right }: VertexOptions) {
    this.id = id;
    this.m_position = new Handle({ position, type: 'vertex', parent: this });

    if (left) this.setLeft(left, true);
    if (right) this.setRight(right, true);
  }

  get position(): vec2 {
    return this.m_position.transform.position;
  }

  get left(): HandleEntity | undefined {
    return this.m_left;
  }

  get right(): HandleEntity | undefined {
    return this.m_right;
  }

  get transform(): SimpleTransformComponent {
    return this.m_position.transform;
  }

  get boundingBox(): Box {
    let min: vec2 = [0, 0];
    let max: vec2 = [0, 0];

    if (this.m_left) {
      min = vec2.min(min, this.m_left.transform.position);
      max = vec2.max(max, this.m_left.transform.position);
    }

    if (this.m_right) {
      min = vec2.min(min, this.m_right.transform.position);
      max = vec2.max(max, this.m_right.transform.position);
    }

    return [vec2.add(min, this.position), vec2.add(max, this.position)] as Box;
  }

  setLeft(position?: vec2 | HandleEntity | null, skipRecordAction: boolean = false): void {
    if (position instanceof Handle) {
      this.m_left = position;
      return;
    }

    if (skipRecordAction) {
      if (position) {
        if (this.m_left) this.m_left.transform.position = position as vec2;
        else this.m_left = new Handle({ position: position as vec2, type: 'bezier', parent: this });
      } else this.m_left = undefined;
      return;
    }

    const backup = this.m_left?.transform.position;

    HistoryManager.record({
      fn: () => {
        this.setLeft(position, true);
      },
      undo: () => {
        this.setLeft(backup, true);
      }
    });
  }

  setRight(position?: vec2 | HandleEntity | null, skipRecordAction: boolean = false): void {
    if (position instanceof Handle) {
      this.m_right = position;
      return;
    }

    if (skipRecordAction) {
      if (position) {
        if (this.m_right) this.m_right.transform.position = position as vec2;
        this.m_right = new Handle({ position: position as vec2, type: 'bezier', parent: this });
      } else this.m_right = undefined;
      return;
    }

    const backup = this.m_right ? this.m_right.transform.position : undefined;

    HistoryManager.record({
      fn: () => {
        this.setRight(position, true);
      },
      undo: () => {
        this.setRight(backup, true);
      }
    });
  }

  mirrorTranslation(id: string): void {
    // const isLeft = this.m_left && this.m_left.id === id;
    // const handle = isLeft ? this.m_left! : this.m_right!;
    // const toMirror = isLeft ? this.m_right : this.m_left;
    // if (!toMirror) return;
    // const direction = vec2.unit(vec2.neg(handle.position));
    // if (!vec2.equals(direction, [0, 0])) {
    //   toMirror.translate(
    //     vec2.sub(vec2.mul(direction, vec2.len(toMirror.position!)), toMirror.position),
    //     true
    //   );
    // }
  }

  applyMirroredTranslation(id: string): void {
    // const isLeft = this.m_left && this.m_left.id === id;
    // const toMirror = isLeft ? this.m_right : this.m_left;
    // if (!toMirror) return;
    // toMirror.applyTransform(true);
  }

  clearMirroredTranslation(id: string): void {
    // const isLeft = this.m_left && this.m_left.id === id;
    // const toMirror = isLeft ? this.m_right : this.m_left;
    // if (!toMirror) return;
    // toMirror.clearTransform(true);
  }

  destroy(): void {}

  getEntityAt(
    position: vec2,
    lowerLevel: boolean = false,
    threshold: number = 0
  ): Entity | undefined {
    if (this.m_position.getEntityAt(position, lowerLevel, threshold)) return this.m_position;

    position = vec2.sub(position, this.position);

    if (lowerLevel && this.m_left && this.m_left.getEntityAt(position, lowerLevel, threshold))
      return this.m_left;

    if (lowerLevel && this.m_right && this.m_right.getEntityAt(position, lowerLevel, threshold))
      return this.m_right;

    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean | undefined): void {
    if (isPointInBox(this.position, box)) entities.add(this);
  }

  getDrawable(useWebGL: boolean = false, selected: boolean = false): Drawable {
    // TODO: optimize operations batching by color
    if (useWebGL) {
      return { operations: [{ type: 'geometry' }] };
    } else {
      const drawable: Drawable = { operations: [{ type: 'begin' }] };
      if (selected) {
        if (this.m_left) {
          drawable.operations.push({
            type: 'circle',
            data: [
              vec2.add(this.m_left.transform.position, this.position),
              3 / SceneManager.viewport.zoom
            ]
          });
        }

        if (this.m_right) {
          drawable.operations.push({
            type: 'circle',
            data: [
              vec2.add(this.m_right.transform.position, this.position),
              3 / SceneManager.viewport.zoom
            ]
          });
        }

        drawable.operations.push({
          type: 'crect',
          data: [this.position, 4 / SceneManager.viewport.zoom]
        });

        drawable.operations.push({ type: 'fill' });
        drawable.operations.push({ type: 'begin' });

        if (this.m_left) {
          drawable.operations.push({
            type: 'move',
            data: [vec2.add(this.m_left.transform.position, this.position)]
          });
          drawable.operations.push({
            type: 'linear',
            data: [this.position]
          });
        }

        if (this.m_right) {
          drawable.operations.push({
            type: 'move',
            data: [vec2.add(this.m_right.transform.position, this.position)]
          });
          drawable.operations.push({
            type: 'linear',
            data: [this.position]
          });
        }
      } else {
        drawable.operations.push({
          type: 'crect',
          data: [this.position, 3 / SceneManager.viewport.zoom]
        });
        drawable.operations.push({ type: 'fillcolor', data: vec4.fromValues(1, 1, 1, 1) });
        drawable.operations.push({ type: 'fill' });
        drawable.operations.push({
          type: 'fillcolor',
          data: vec4.fromValues(49 / 255, 239 / 255, 284 / 255, 1)
        });
      }

      drawable.operations.push({ type: 'stroke' });
      return drawable;
    }
  }

  getOutlineDrawable(useWebGL: boolean = false): Drawable {
    return { operations: [] };
  }

  render(selected: boolean = false): void {
    Renderer.draw(this.getDrawable(false, !!selected));
  }

  asObject(duplicate: boolean = false): VertexObject {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      position: this.position,
      left: this.m_left?.transform.position,
      right: this.m_right?.transform.position
    };
  }

  toJSON(): VertexObject {
    return this.asObject(false);
  }
}

export default Vertex;
