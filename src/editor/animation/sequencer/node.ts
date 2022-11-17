import { FloatValue, MapValue, Vec2Value } from '@/editor/history/value';
import { vec2 } from '@/math';
import { nanoid } from 'nanoid';

export class Node implements SequenceNode {
  readonly id: string;

  readonly position: Vec2Value;
  readonly duration: FloatValue;

  private m_size: vec2 = vec2.create();
  private m_links: MapValue<string, SequenceNode> = new MapValue();

  private m_now: number = 0;
  private m_animating: boolean = false;

  constructor({
    id = nanoid(),
    position = [100, 100],
    duration = 1000,
    links = []
  }: SequencerNodeOptions) {
    this.id = id;
    this.position = new Vec2Value(position);
    this.duration = new FloatValue(duration);
  }

  get size(): vec2 {
    return vec2.clone(this.m_size);
  }

  set size(value: vec2) {
    vec2.copy(this.m_size, value);
  }

  get percent(): number {
    return this.m_now / this.duration.value;
  }

  get animating(): boolean {
    return this.m_animating;
  }

  get boundingBox(): Box {
    const position = this.position.value;
    return [position, vec2.add(position, this.m_size)];
  }

  link(node: SequenceNode): void {
    this.m_links.set(node.id, node);
  }

  unlink(id: string): void {
    this.m_links.delete(id);
  }

  forEach(callback: (link: SequenceNode) => void): void {
    this.m_links.forEach((link) => callback(link));
  }

  animate(interval: number): AnimationReturnState {
    this.m_now += interval;

    if (this.m_now >= this.duration.value) {
      this.reset();

      const playing: string[] = [];

      this.m_links.forEach((link) => {
        playing.push(link.id);
        link.animate(interval);
      });

      return { playing, remove: true };
    }

    this.m_animating = true;

    return { playing: [], remove: false };
  }

  reset(): void {
    this.m_animating = false;
    this.m_now = 0;
  }
}
