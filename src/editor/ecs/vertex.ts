import { isPointInBox, vec2, vec4 } from '@math';
import { nanoid } from 'nanoid';
import HistoryManager from '../history';
import { Renderer } from '../renderer';
import SceneManager from '../scene';
import Element from './element';
import Handle from './handle';

class Vertex implements VertexEntity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'vertex';

  public parent: Element;

  private m_position: Handle;
  private m_left?: Handle;
  private m_right?: Handle;

  constructor({ id = nanoid(), position, left, right }: VertexOptions) {
    this.id = id;
    this.m_position = new Handle({ position, type: 'vertex', parent: this });

    if (left) this.setLeft(left, true);
    if (right) this.setRight(right, true);
  }

  public get visible() {
    return true;
  }

  public get position() {
    return this.m_position.position;
  }

  public set position(position: vec2) {
    this.m_position.position = position;
  }

  public get left() {
    return this.m_left;
  }

  public get right() {
    return this.m_right;
  }

  public get boundingBox() {
    let min: vec2 = [0, 0];
    let max: vec2 = [0, 0];

    if (this.m_left) {
      min = vec2.min(min, this.m_left.position);
      max = vec2.max(max, this.m_left.position);
    }

    if (this.m_right) {
      min = vec2.min(min, this.m_right.position);
      max = vec2.max(max, this.m_right.position);
    }

    return [vec2.add(min, this.position), vec2.add(max, this.position)] as Box;
  }

  public move(delta: vec2) {
    this.m_position.move(delta);
  }

  public moveTo(position: vec2) {
    this.m_position.moveTo(position);
  }

  public translate(delta: vec2) {
    this.m_position.translate(delta);
  }

  public applyTransform() {
    this.m_position.applyTransform();
  }
  public clearTransform() {
    this.m_position.clearTransform();
  }

  public mirrorTranslation(id: string) {
    const isLeft = this.m_left && this.m_left.id === id;
    const handle = isLeft ? this.m_left! : this.m_right!;
    const toMirror = isLeft ? this.m_right : this.m_left;

    if (!toMirror) return;

    const direction = vec2.unit(vec2.neg(handle.position));

    if (!vec2.equals(direction, [0, 0]))
      toMirror.translate(
        vec2.sub(vec2.mul(direction, vec2.len(toMirror.position!)), toMirror.position),
        true
      );
  }

  public applyMirroredTranslation(id: string) {
    const isLeft = this.m_left && this.m_left.id === id;
    const toMirror = isLeft ? this.m_right : this.m_left;

    if (!toMirror) return;

    toMirror.applyTransform(true);
  }

  public clearMirroredTranslation(id: string) {
    const isLeft = this.m_left && this.m_left.id === id;
    const toMirror = isLeft ? this.m_right : this.m_left;

    if (!toMirror) return;

    toMirror.clearTransform(true);
  }

  public setLeft(position?: vec2 | null | Handle, skipRecordAction = false) {
    if (position instanceof Handle) {
      this.m_left = position;
      return;
    }

    if (skipRecordAction) {
      if (position) {
        if (this.m_left) this.m_left.position = position;
        else this.m_left = new Handle({ position, type: 'bezier', parent: this });
      } else this.m_left = undefined;

      return;
    }

    const backup = this.m_left ? this.m_left.position : undefined;

    HistoryManager.record({
      fn: () => {
        this.setLeft(position, true);
      },
      undo: () => {
        this.setLeft(backup, true);
      }
    });
  }

  public setRight(position?: vec2 | null | Handle, skipRecordAction = false) {
    if (position instanceof Handle) {
      this.m_right = position;
      return;
    }

    if (skipRecordAction) {
      if (position) {
        if (this.m_right) this.m_right.position = position;
        this.m_right = new Handle({ position, type: 'bezier', parent: this });
      } else this.m_right = undefined;

      return;
    }

    const backup = this.m_right ? this.m_right.position : undefined;

    HistoryManager.record({
      fn: () => {
        this.setRight(position, true);
      },
      undo: () => {
        this.setRight(backup, true);
      }
    });
  }

  public getEntityAt(position: vec2, lowerLevel = false, threshold: number = 0) {
    if (this.m_position.getEntityAt(position, lowerLevel, threshold)) return this.m_position;

    position = vec2.sub(position, this.position);

    if (lowerLevel && this.m_left && this.m_left.getEntityAt(position, lowerLevel, threshold))
      return this.m_left;

    if (lowerLevel && this.m_right && this.m_right.getEntityAt(position, lowerLevel, threshold))
      return this.m_right;

    return undefined;
  }

  public getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean): void {
    if (isPointInBox(this.position, box)) entities.add(this);
  }

  public delete() {}

  public deleteSelf() {}

  public render(selected?: boolean) {
    Renderer.draw(this.getDrawable(!!selected));
  }

  public getDrawable(selected: boolean, useWebGL = false): Drawable {
    // TODO: optimize operations batching by color
    if (useWebGL) {
      return { operations: [{ type: 'geometry' }] };
    } else {
      const drawable: Drawable = { operations: [{ type: 'begin' }] };
      if (selected) {
        if (this.m_left) {
          drawable.operations.push({
            type: 'circle',
            data: [vec2.add(this.m_left.position, this.position), 3 / SceneManager.viewport.zoom]
          });
        }

        if (this.m_right) {
          drawable.operations.push({
            type: 'circle',
            data: [vec2.add(this.m_right.position, this.position), 3 / SceneManager.viewport.zoom]
          });
        }

        drawable.operations.push({
          type: 'rect',
          data: [this.position, 4 / SceneManager.viewport.zoom]
        });

        drawable.operations.push({ type: 'fill' });
        drawable.operations.push({ type: 'begin' });

        if (this.m_left) {
          drawable.operations.push({
            type: 'move',
            data: [vec2.add(this.m_left.position, this.position)]
          });
          drawable.operations.push({
            type: 'linear',
            data: [this.position]
          });
        }

        if (this.m_right) {
          drawable.operations.push({
            type: 'move',
            data: [vec2.add(this.m_right.position, this.position)]
          });
          drawable.operations.push({
            type: 'linear',
            data: [this.position]
          });
        }
      } else {
        drawable.operations.push({
          type: 'rect',
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

  public toJSON(duplicate = false) {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      position: vec2.clone(this.position),
      left: this.m_left ? vec2.clone(this.m_left.position) : undefined,
      right: this.m_right ? vec2.clone(this.m_right.position) : undefined
    };
  }
}

export default Vertex;
