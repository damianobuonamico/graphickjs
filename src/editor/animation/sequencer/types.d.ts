interface AnimationReturnState {
  playing: string[];
  remove: boolean;
}

interface SequenceNode {
  readonly id: string;
  readonly position: Value<vec2>;
  readonly duration: Value<number>;
  readonly color: Value<string>;

  size: vec2;

  readonly percent: number;
  readonly animating: boolean;
  readonly boundingBox: Box;

  link(node: SequenceNode): void;
  unlink(id: string): void;

  forEach(callback: (link: SequenceNode) => void): void;
  animate(interval: number): AnimationReturnState;

  ready(entities: Set<string>): void;
  reset(): void;
}

interface SequencerNodeOptions {
  entity?: Entity;
  animation?: AnimationInterface;
  position?: vec2;
  duration?: number;
  links?: SequenceNode[];
}

interface AnimationInterface {
  name: string;
  readonly id: string;

  ready(entity: Entity): void;
  animate(entity: Entity, t: number, duration: number): void;
  reset(entity: Entity): void;
}
