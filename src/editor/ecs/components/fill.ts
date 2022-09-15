import SceneManager from '@/editor/scene';
import Color from '@utils/color';
import { nanoid } from 'nanoid';
import Element from '../element';

class Fill {
  public readonly id: string;

  public style: 'solid';

  private m_color: Color;
  private m_parents: Set<Element> = new Set();

  constructor({ id = nanoid(), style = 'solid', color = [1, 1, 1, 1] }: FillOptions) {
    this.id = id;
    this.style = style;
    this.m_color = new Color(color);
  }

  public get color() {
    return this.m_color.vec4;
  }

  public addParent(parent: Element) {
    this.m_parents.add(parent);
  }

  public deleteParent(parent: Element) {
    this.m_parents.delete(parent);

    if (this.m_parents.size === 0) {
      SceneManager.assets.delete(this.id, 'fill');
    }
  }

  public toJSON(duplicate = false) {
    const obj: FillOptions = {
      id: duplicate ? nanoid() : this.id,
      color: this.color
    };

    if (this.style !== 'solid') obj.style = this.style;

    return obj;
  }
}

export default Fill;
