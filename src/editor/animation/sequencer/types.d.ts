interface AnimationReturnState {
  playing: string[];
  remove: boolean;
}

interface SequenceNode {
  readonly id: string;
  readonly position: Value<vec2>;

  size: vec2;
  duration: number;

  readonly percent: number;
  readonly animating: boolean;
  readonly boundingBox: Box;

  link(node: SequenceNode): void;
  unlink(id: string): void;

  forEach(callback: (link: SequenceNode) => void): void;
  animate(interval: number): AnimationReturnState;

  reset(): void;
}

interface SequencerNodeOptions {
  id?: string;
  position?: vec2;
  duration?: number;
  links?: SequenceNode[];
}

interface EntrySequencerNode extends SequenceNode {
  readonly color: '#4D715B';
  readonly name: string;
}

interface EntitySequencerNode extends SequenceNode {
  readonly entity: Entity;

  color: string;
  name: string;
}

interface EntitySequencerNodeOptions extends SequencerNodeOptions {
  color?: string;
  name?: string;
}
