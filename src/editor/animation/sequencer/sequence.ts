import Node from './nodes/node';

class Sequence {
  private m_entry: SequenceNode;
  private m_nodes: SequenceNode[];
  private m_exit: SequenceNode;

  constructor() {
    this.m_entry = new Node([100, 100], 'Entry', '#4D715B');
    this.m_nodes = [new Node([250, 100])];
    this.m_exit = new Node([400, 100], 'Exit', '#7C4747');
  }

  forEach(callback: (node: SequenceNode) => any) {
    callback(this.m_entry);

    for (let i = 0, n = this.m_nodes.length; i < n; ++i) callback(this.m_nodes[i]);

    callback(this.m_exit);
  }
}

export default Sequence;
