import SceneManager from '@/editor/scene';
import Color from '@utils/color';
import { nanoid } from 'nanoid';
import Element from '../element';

class Stroke implements Stroke {
  public readonly id: string;

  public style: 'solid' | number[] | null = 'solid';
  public width: number = 1;
  public cap: CanvasLineCap = 'butt';
  public corner: CanvasLineJoin = 'miter';
  public miterLimit: number = 10;
  public color: Color;

  private m_parents: Set<Element> = new Set();

  constructor({
    id = nanoid(),
    style = 'solid',
    width = 1,
    cap = 'butt',
    corner = 'miter',
    miterLimit = 10,
    color = [0, 0, 0, 1]
  }: StrokeOptions) {
    this.id = id;
    this.style = style;
    this.width = width;
    this.cap = cap;
    this.corner = corner;
    this.miterLimit = miterLimit;
    this.color = new Color(color);
  }

  public addParent(parent: Element) {
    this.m_parents.add(parent);
  }

  public deleteParent(parent: Element) {
    this.m_parents.delete(parent);

    if (this.m_parents.size === 0) {
      SceneManager.assets.delete(this.id, 'stroke');
    }
  }

  public toJSON(duplicate = false) {
    const obj: StrokeOptions = {
      id: duplicate ? nanoid() : this.id,
      color: this.color.vec4
    };

    if (this.style !== 'solid') obj.style = this.style;
    if (this.width !== 1) obj.width = this.width;
    if (this.cap !== 'butt') obj.cap = this.cap;
    if (this.corner !== 'miter') obj.corner = this.corner;
    if (this.miterLimit !== 10) obj.miterLimit = this.miterLimit;

    return obj;
  }
}

export default Stroke;
