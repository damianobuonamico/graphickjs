interface Entity {
  readonly id: string;
  readonly type: 'artboard' | 'layer' | 'element' | 'vertex' | 'handle';

  parent: Entity;

  render(): void;
}

interface VertexEntity extends Entity {}

interface ElementOptions {
  id?: string;
  position: vec2;
  vertices?: VertexEntity[];
}

interface VertexOptions {
  id?: string;
  position: vec2;
  left?: vec2;
  right?: vec2;
}

interface HandleOptions {
  position: vec2;
  type: 'vertex' | 'bezier';
  parent: Entity;
}
