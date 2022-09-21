import SceneManager from '@/editor/scene';
import Color from '@utils/color';
import { nanoid } from 'nanoid';
import Element from '../entities/element';

class Fill implements Fill {
  public readonly id: string;

  public style: 'solid';
  public color: Color;

  private m_parents: Set<Element> = new Set();

  constructor({ id = nanoid(), style = 'solid', color = [1, 1, 1, 1] }: FillOptions) {
    this.id = id;
    this.style = style;
    this.color = new Color(color);
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
      color: this.color.vec4
    };

    if (this.style !== 'solid') obj.style = this.style;

    return obj;
  }
}

export default Fill;
