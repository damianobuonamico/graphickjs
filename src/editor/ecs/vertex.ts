import { vec2 } from '@math';
import { nanoid } from 'nanoid';
import { Renderer } from '../renderer';
import Handle from './handle';

class Vertex implements VertexEntity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'vertex';
  public parent: Entity;

  private m_position: Handle;
  private m_left?: Handle;
  private m_right?: Handle;

  constructor({ id = nanoid(), position, left, right }: VertexOptions) {
    this.id = id;
    this.m_position = new Handle({ position, type: 'vertex', parent: this });
    if (left) this.setLeft(left);
    if (right) this.setRight(right);
  }

  public get position() {
    return this.m_position.position;
  }

  public get left() {
    return this.m_left;
  }

  public setLeft(position?: vec2 | null) {
    if (position) {
      if (this.m_left) this.m_left.position = position;
      else this.m_left = new Handle({ position, type: 'bezier', parent: this });
    } else this.m_left = undefined;
  }

  public get right() {
    return this.m_right;
  }

  public setRight(position?: vec2 | null) {
    if (position) {
      if (this.m_right) this.m_right.position = position;
      this.m_right = new Handle({ position, type: 'bezier', parent: this });
    } else this.m_right = undefined;
  }

  public get visible() {
    return true;
  }

  public translate(delta: vec2) {
    this.m_position.translate(delta);
  }

  delete() {}

  render() {
    Renderer.rect({
      pos: this.position,
      size: [4, 4],
      color: [1.0, 0.0, 1.0, 1.0],
      centered: true
    });
    if (this.m_left)
      Renderer.rect({
        pos: vec2.add(this.m_left.position, this.position),
        size: [4, 4],
        color: [1.0, 0.0, 0.0, 1.0],
        centered: true
      });
    if (this.m_right)
      Renderer.rect({
        pos: vec2.add(this.m_right.position, this.position),
        size: [4, 4],
        color: [1.0, 0.0, 0.0, 1.0],
        centered: true
      });
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

  public getEntityAt(position: vec2, threshold: number = 0) {
    let toReturn = undefined;
    [this.m_position, this.m_left, this.m_right].forEach((handle) => {
      if (handle) {
        const result = handle.getEntityAt(position, threshold);
        if (result) toReturn = result;
      }
    });
    return toReturn;
  }

  public getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean | undefined): void {}

  public applyTransform() {}
}

export default Vertex;
