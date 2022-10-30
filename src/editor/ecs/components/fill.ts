import Color from '@/editor/ecs/components/color';
import { nanoid } from 'nanoid';

class Fill implements FillComponent {
  public readonly id: string;

  public style: FillComponent['style'];
  public color: ColorComponent;

  constructor({ id = nanoid(), style = 'solid', color = [1, 1, 1, 1] }: FillOptions) {
    this.id = id;
    this.style = style;
    this.color = new Color(color);
  }

  public asObject(duplicate = false) {
    const obj: FillComponentObject = {
      id: duplicate ? nanoid() : this.id,
      color: this.color.vec4
    };

    if (this.style !== 'solid') obj.style = this.style;

    return obj;
  }
}

export default Fill;
