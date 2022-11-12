import { vec2 } from '@/math';

class Node implements SequenceNode {
  size: vec2 = [100, 50];
  position: vec2;

  name: string = 'Node';
  color = '#30363D';

  constructor(position: vec2, name?: string, color?: string) {
    this.position = vec2.clone(position);

    if (name) this.name = name;
    if (color) this.color = color;
  }

  get boundingBox(): Box {
    return [vec2.clone(this.position), vec2.add(this.position, this.size)];
  }
}

export default Node;
