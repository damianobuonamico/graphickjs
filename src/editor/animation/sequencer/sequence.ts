import { isElement } from '@/editor/ecs/entities/element';
import EntityNode from './nodes/entityNode';
import Node from './nodes/node';

class Sequence {
  private m_entry: SequenceNode;
  private m_nodes: EntityNode[];
  private m_exit: SequenceNode;

  private m_index: number = 0;
  private m_ms: number = 0;

  constructor() {
    this.m_entry = new Node([100, 100], 'Entry', '#4D715B');
    this.m_nodes = [];
    this.m_exit = new Node([400, 100], 'Exit', '#7C4747');
  }

  forEach(callback: (node: SequenceNode) => any) {
    callback(this.m_entry);

    for (let i = 0, n = this.m_nodes.length; i < n; ++i) callback(this.m_nodes[i]);

    callback(this.m_exit);
  }

  add(entity: Entity) {
    this.m_nodes.push(new EntityNode(entity, [250, 100]));
  }

  animate(fps: number): void {
    const node = this.m_nodes[this.m_index];
    if (!node || !(node instanceof EntityNode)) {
      this.m_index = 0;
      this.m_ms = 0;

      return;
    }

    if (this.m_ms > node.duration) {
      this.m_index++;
      this.m_ms = 0;
      return this.animate(fps);
    }

    const percent = this.m_ms / node.duration;
    const movement = 100;

    if (isElement(node.entity))
      node.entity.transform.tempRotate((2 * Math.PI * (1000 / fps)) / node.duration);

    // if (node.entity.opacity !== undefined) node.entity.opacity = percent;

    this.m_ms += 1000 / fps;
  }

  toJSON() {
    return { nodes: this.m_nodes.map((node) => node.entity.id) };
  }

  load(sequence: Entity[]) {
    sequence.forEach((entity) => this.add(entity));
  }
}

export default Sequence;
