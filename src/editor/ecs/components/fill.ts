import Color from '@/editor/ecs/components/color';
import { nanoid } from 'nanoid';

class Fill implements FillComponent {
  public readonly id: string;

  public style: FillComponent['style'];
  public color: ColorComponent;

  private m_visible: boolean;

  constructor({
    id = nanoid(),
    style = 'solid',
    color = [1, 1, 1, 1],
    visible = true
  }: FillOptions) {
    this.id = id;
    this.style = style;
    this.color = new Color(color);
    this.m_visible = visible;
  }

  public asObject(duplicate = false) {
    const obj: FillComponentObject = {
      id: duplicate ? nanoid() : this.id,
      color: this.color.vec4
    };

    if (this.style !== 'solid') obj.style = this.style;
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
}

export default Fill;
