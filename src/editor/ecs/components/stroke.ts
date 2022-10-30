import Color from '@/editor/ecs/components/color';
import { nanoid } from 'nanoid';

class Stroke implements StrokeComponent {
  public readonly id: string;

  public style: StrokeComponent['style'];
  public width: number = 1;
  public cap: CanvasLineCap = 'butt';
  public corner: CanvasLineJoin = 'miter';
  public miterLimit: number = 10;
  public color: ColorComponent;

  constructor({ id = nanoid(), style = 'solid', color = [1, 1, 1, 1] }: StrokeOptions) {
    this.id = id;
    this.style = style;
    this.color = new Color(color);
  }

  public asObject(duplicate = false) {
    const obj: StrokeComponentObject = {
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
