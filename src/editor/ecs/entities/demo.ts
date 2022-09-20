import { Renderer } from '@/editor/renderer';
import { doesBoxIntersectBox, isPointInBox, vec2 } from '@/math';
import { nanoid } from 'nanoid';
import { Transform } from '../components/transform';

class Demo implements DemoEntity {
  public id;
  public readonly type = 'demo';

  public readonly transform = new Transform();

  constructor({ id = nanoid() }: DemoOptions) {
    this.id = id;
  }

  public get boundingBox() {
    return [this.transform.position, vec2.add(this.transform.position, [100, 100])] as Box;
  }

  getEntityAt(position: vec2, lowerLevel = false, threshold: number = 0) {
    if (isPointInBox(position, this.boundingBox, threshold)) return this as any;
    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>) {
    if (doesBoxIntersectBox(box, this.boundingBox)) entities.add(this as any);
  }

  getOutlineDrawable(): Drawable {
    return {
      operations: [
        {
          type: 'rect',
          data: [this.transform.position, [100, 100]]
        },
        { type: 'stroke' }
      ]
    };
  }

  render() {
    Renderer.rect({
      pos: this.transform.position,
      size: [100, 100],
      color: [1.0, 0.0, 0.0, 1.0]
    });
  }

  toJSON() {
    return {};
  }
}

export default Demo;
