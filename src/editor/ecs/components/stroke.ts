import Color from '@/editor/ecs/components/color';
import HistoryManager from '@/editor/history';
import { nanoid } from 'nanoid';

class Stroke implements StrokeComponent {
  public readonly id: string;

  public style: StrokeComponent['style'];
  public width: number = 1;
  public cap: CanvasLineCap = 'butt';
  public corner: CanvasLineJoin = 'miter';
  public miterLimit: number = 10;
  public color: ColorComponent;

  private m_visible: boolean;

  constructor({
    id = nanoid(),
    style = 'solid',
    color = [1, 1, 1, 1],
    visible = true
  }: StrokeOptions) {
    this.id = id;
    this.style = style;
    this.color = new Color(color);
    this.m_visible = visible;
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
    if (this.m_visible === false) obj.visible = false;

    return obj;
  }

  get visible() {
    return this.m_visible;
  }

  set visible(value: boolean) {
    const backup = this.m_visible;
    if (value === backup) return;

    HistoryManager.record({
      fn: () => {
        this.m_visible = value;
      },
      undo: () => {
        this.m_visible = backup;
      }
    });
  }

  set tempVisible(value: boolean) {
    this.m_visible = value;
  }
}

export default Stroke;
