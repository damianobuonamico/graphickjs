import CommandHistory from '@/editor/history/history';
import { MapValue } from '@/editor/history/value';
import { FadeInAnimation } from '../animations/fadeIn';
import { Node } from './node';

class Sequence {
  private m_entry: SequenceNode;
  private m_nodes: MapValue<string, SequenceNode> = new MapValue();

  private m_playing: Set<string> = new Set();
  private m_animations = {
    fadeIn: new FadeInAnimation()
  };

  constructor() {
    this.m_entry = new Node({});
    CommandHistory.ignoreNext();
    this.m_entry.color.value = '#4D715B';
  }

  add(entity: Entity) {
    const node = new Node({ entity, animation: this.m_animations.fadeIn });
    this.m_nodes.set(node.id, node);
  }

  link(a: string, b: string) {
    const parent = this.m_nodes.get(a);
    const node = this.m_nodes.get(b);

    if (parent && node) parent.link(node);
  }

  forEach(callback: (node: SequenceNode) => any) {
    callback(this.m_entry);

    this.m_nodes.forEach((node) => callback(node));
  }

  forEachReversed(callback: (node: SequenceNode) => any) {
    Array.from(this.m_nodes.values())
      .reverse()
      .forEach((node) => callback(node));

    callback(this.m_entry);
  }

  animate(fps: number): void {
    if (this.m_playing.size === 0) {
      const entities = new Set<string>();
      this.m_entry.ready(entities);
      this.m_playing = new Set(this.m_entry.animate(fps).playing);
      return;
    }

    this.m_playing.forEach((id) => {
      const node = id === this.m_entry.id ? this.m_entry : this.m_nodes.get(id);
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
