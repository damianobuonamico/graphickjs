import { nanoid } from 'nanoid';

class Element implements Entity {
  public readonly id: string;

  constructor() {
    this.id = nanoid();
  }

  public render() {}
}

export default Element;
