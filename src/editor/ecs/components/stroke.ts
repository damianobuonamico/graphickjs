import Color from '@/editor/ecs/components/color';
import { FloatValue } from '@/editor/history/value';
import { nanoid } from 'nanoid';

// TODO: refactor getters with value direct access
class Stroke implements StrokeComponent {
  public readonly id: string;

  public style: StrokeComponent['style'];
  public cap: CanvasLineCap = 'butt';
  public corner: CanvasLineJoin = 'miter';
  public miterLimit: number = 10;
  public color: ColorComponent;

  private m_visible: boolean;
  private m_width: FloatValue;

  constructor({
    id = nanoid(),
    style = 'solid',
    color = [1, 1, 1, 1],
    width = 1,
    corner = 'miter',
    visible = true
  }: StrokeOptions) {
    this.id = id;
    this.style = style;
    this.color = new Color(color);
    this.m_visible = visible;
    this.m_width = new FloatValue(width);
    this.corner = corner;
  }

  public asObject(duplicate = false) {
    const obj: StrokeComponentObject = {
      id: duplicate ? nanoid() : this.id,
      color: this.color.vec4
    };

    if (this.style !== 'solid') obj.style = this.style;
    if (this.width !== 1) obj.width = this.m_width.value;
    if (this.cap !== 'butt') obj.cap = this.cap;
    if (this.corner !== 'miter') obj.corner = this.corner;
    if (this.miterLimit !== 10) obj.miterLimit = this.miterLimit;
    if (this.m_visible === false) obj.visible = false;

    return obj;
  }

  get visible() {
    return this.m_visible;
  }

  set visible(value: boolean) {
    const backup = this.m_visible;
    if (value === backup) return;

    // TOCHECK
    this.m_visible = value;

    // this.m_visible = backup;
  }

  set tempVisible(value: boolean) {
    this.m_visible = value;
  }

  get width() {
    return this.m_width.value;
  }

  set width(value: number) {
    this.m_width.value = typeof value === 'string' ? parseInt(value) : value;
  }
}

export default Stroke;
