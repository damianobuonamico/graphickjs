import { vec2 } from '@/math';
import { nanoid } from 'nanoid';

class GraphNode {
  readonly id: string;
  readonly entity: Entity;

  size = vec2.create();
  position = [100, 100];
  color = '#30363D';

  duration = 1000;
  percent = 0;
  animating = false;
  private m_now = 0;

  private m_links: Map<string, GraphNode> = new Map();

  constructor(entity: Entity) {
    this.id = nanoid();
    this.entity = entity;
  }

  get name() {
    return this.id;
  }

  get boundingBox(): Box {
    return [vec2.clone(this.position), vec2.add(this.position, this.size)];
  }

  link(node: GraphNode) {
    this.m_links.set(node.id, node);
  }

  unlink(id: string) {
    this.m_links.delete(id);
  }

  forEach(callback: (link: GraphNode) => void) {
    this.m_links.forEach((link) => callback(link));
  }

  animate(fps: number): { playing: string[]; remove: boolean } {
    this.m_now += 1000 / fps;

    if (this.m_now >= this.duration) {
      this.reset();

      const playing: string[] = [];

      this.m_links.forEach((link) => {
        playing.push(link.id);
        link.animate(fps);
      });

      return { playing, remove: true };
    }

    this.animating = true;
    this.percent = this.m_now / this.duration;

    return { playing: [], remove: false };
  }

  reset() {
    this.animating = false;
    this.m_now = 0;
    this.percent = 0;
  }
}

class Sequence {
  private m_entry: GraphNode = new GraphNode(undefined);
  private m_nodes: Map<string, GraphNode> = new Map();

  private m_playing: Set<string> = new Set();

  add(entity: Entity) {
    const node = new GraphNode(entity);
    this.m_nodes.set(node.id, node);
    console.log(node);
  }

  link(a: string, b: string) {
    const parent = this.m_nodes.get(a);
    const node = this.m_nodes.get(b);

    if (parent && node) parent.link(node);
  }

  forEach(callback: (node: GraphNode) => any) {
    callback(this.m_entry);

    this.m_nodes.forEach((node) => callback(node));
  }

  forEachReversed(callback: (node: GraphNode) => any) {
    Array.from(this.m_nodes.values())
      .reverse()
      .forEach((node) => callback(node));

    callback(this.m_entry);
  }

  animate(fps: number): void {
    if (this.m_playing.size === 0) {
      console.log('start');
      this.m_playing = new Set(this.m_entry.animate(fps).playing);
      return;
    }

    this.m_playing.forEach((id) => {
      const node = this.m_nodes.get(id);
      if (!node) return this.m_playing.delete(id);

      const result = node.animate(fps);

      result.playing.forEach((n) => this.m_playing.add(n));

      if (result.remove) this.m_playing.delete(id);
    });
  }

  stop() {
    this.forEach((node) => node.reset());
    this.m_playing.clear();
  }

  toJSON() {
    return {};
  }

  load(sequence: Entity[]) {}
}

export default Sequence;
