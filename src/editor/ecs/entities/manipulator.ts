import InputManager from '@/editor/input';
import { Renderer } from '@/editor/renderer';
import SceneManager from '@/editor/scene';
import { isPointInCircle, vec2 } from '@/math';
import { nanoid } from 'nanoid';
import { UntrackedSimpleTransform, UntrackedTransform } from '../components/transform';

class GenericHandle implements Entity {
  transform: UntrackedSimpleTransform;
  boundingBox: Box;
  readonly id: string;
  readonly selectable = false;
  type: EntityType = 'generichandle';
  parent: Entity;
  handleType: string;

  private m_radius: number;

  constructor({ type, id = nanoid(), position }: { type: string; id?: string; position?: vec2 }) {
    this.handleType = type;
    this.id = id;
    this.m_radius = type === 'rotate' ? 9 : 3;
    this.transform = new UntrackedSimpleTransform(position);
  }

  destroy(): void {}

  getEntityAt(
    position: vec2,
    lowerLevel: boolean = false,
    threshold: number = 0
  ): Entity | undefined {
    if (
      isPointInCircle(
        position,
        this.transform.position,
        (this.m_radius + 2) / SceneManager.viewport.zoom
      )
    )
      return this;
    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean | undefined): void {}

  getDrawable(useWebGL: boolean = false): Drawable {
    return {
      operations: [
        {
          type: 'rect',
          data: [
            vec2.add(
              this.transform.position,
              vec2.divS([-this.m_radius, -this.m_radius], SceneManager.viewport.zoom)
            ),
            vec2.divS([this.m_radius * 2, this.m_radius * 2], SceneManager.viewport.zoom)
          ]
        },
        { type: 'stroke' }
      ]
    };
  }

  getOutlineDrawable(useWebGL: boolean = false): Drawable {
    return this.getDrawable(useWebGL);
  }

  render(): void {}

  asObject(duplicate?: boolean | undefined): EntityObject {
    throw new Error('Method not implemented.');
  }

  toJSON(): EntityObject {
    throw new Error('Method not implemented.');
  }
}

interface TransformHandles<T> {
  n: T;
  s: T;
  e: T;
  w: T;
  nw: T;
  ne: T;
  sw: T;
  se: T;
  rn: T;
  rs: T;
  re: T;
  rw: T;
  rnw: T;
  rne: T;
  rsw: T;
  rse: T;
}

type HandleKey = keyof TransformHandles<null>;

class Manipulator implements ManipulatorEntity {
  readonly id: string = nanoid();
  readonly type: EntityType = 'manipulator';
  readonly selectable = false;

  parent: Entity;
  transform: UntrackedTransformComponent;

  private m_lastCalculatedSize: vec2 = vec2.create();
  private m_active: boolean = false;
  private m_handles: TransformHandles<GenericHandle> = {
    n: new GenericHandle({ type: 'scale', id: 'scale-n' }),
    s: new GenericHandle({ type: 'scale', id: 'scale-s' }),
    e: new GenericHandle({ type: 'scale', id: 'scale-e' }),
    w: new GenericHandle({ type: 'scale', id: 'scale-w' }),
    nw: new GenericHandle({ type: 'scale', id: 'scale-nw' }),
    ne: new GenericHandle({ type: 'scale', id: 'scale-ne' }),
    sw: new GenericHandle({ type: 'scale', id: 'scale-sw' }),
    se: new GenericHandle({ type: 'scale', id: 'scale-se' }),
    rn: new GenericHandle({ type: 'rotate', id: 'rotate-n' }),
    rs: new GenericHandle({ type: 'rotate', id: 'rotate-s' }),
    re: new GenericHandle({ type: 'rotate', id: 'rotate-e' }),
    rw: new GenericHandle({ type: 'rotate', id: 'rotate-w' }),
    rnw: new GenericHandle({ type: 'rotate', id: 'rotate-nw' }),
    rne: new GenericHandle({ type: 'rotate', id: 'rotate-ne' }),
    rsw: new GenericHandle({ type: 'rotate', id: 'rotate-sw' }),
    rse: new GenericHandle({ type: 'rotate', id: 'rotate-se' })
  };
  private m_positions: TransformHandles<(size: vec2) => vec2> = {
    n: (size: vec2) => [size[0] / 2, 0],
    s: (size: vec2) => [size[0] / 2, size[1]],
    e: (size: vec2) => [size[0], size[1] / 2],
    w: (size: vec2) => [0, size[1] / 2],
    nw: (size: vec2) => [0, 0],
    ne: (size: vec2) => [size[0], 0],
    sw: (size: vec2) => [0, size[1]],
    se: (size: vec2) => size,
    rn: (size: vec2) => [size[0] / 2, 0],
    rs: (size: vec2) => [size[0] / 2, size[1]],
    re: (size: vec2) => [size[0], size[1] / 2],
    rw: (size: vec2) => [0, size[1] / 2],
    rnw: (size: vec2) => [0, 0],
    rne: (size: vec2) => [size[0], 0],
    rsw: (size: vec2) => [0, size[1]],
    rse: (size: vec2) => size
  };
  private m_offsets: Partial<TransformHandles<vec2>> = {
    rn: [0, -6],
    rs: [0, 6],
    re: [6, 0],
    rw: [-6, 0],
    rnw: [-6, -6],
    rne: [6, -6],
    rsw: [-6, 6],
    rse: [6, 6]
  };

  constructor() {
    this.transform = new UntrackedTransform();
  }

  get boundingBox(): Box {
    return [this.transform.position, vec2.add(this.transform.position, this.m_lastCalculatedSize)];
  }

  get active() {
    return this.m_active && !InputManager.tool.isVertex;
  }

  private setHandles(size: vec2 = this.transform.size) {
    this.m_lastCalculatedSize = size;

    Object.keys(this.m_handles).forEach((key) => {
      this.m_handles[key as HandleKey].transform.position =
        this.m_positions[key as HandleKey](size);
      if (this.m_handles[key as HandleKey].handleType === 'rotate')
        this.m_handles[key as HandleKey].transform.translate(
          vec2.divS(this.m_offsets[key as HandleKey] ?? [0, 0], SceneManager.viewport.zoom)
        );
    });
  }

  set(box: Box | null, angle: number = 0): void {
    if (!box) {
      this.m_active = false;
      return;
    }

    this.m_active = true;

    this.transform.position = box[0];
    this.transform.rotation = angle;
    this.transform.size = vec2.sub(box[1], box[0]);

    this.setHandles();

    if (!SceneManager.overlays.has(this.id))
      SceneManager.overlays.add({
        entity: this,
        condition: () => this.active,
        interactive: true,
        static: true
      });
  }

  destroy(): void {}

  getEntityAt(
    position: vec2,
    lowerLevel?: boolean | undefined,
    threshold?: number | undefined
  ): Entity | undefined {
    if (!this.active) return undefined;
    position = vec2.sub(position, this.transform.position);

    if (this.transform.rotation !== 0)
      position = vec2.rotate(position, vec2.divS(this.transform.size, 2), -this.transform.rotation);

    let toReturn: Entity | undefined;

    const size = vec2.mulS(this.transform.size, SceneManager.viewport.zoom);
    const isBoxSmall = [size[0] < 30, size[1] < 30];

    Object.entries(this.m_handles).forEach(([key, handle]: [string, GenericHandle]) => {
      if (!toReturn) {
        let toCheck = true;

        if ((key === 'n' || key === 's' || key === 'rn' || key === 'rs') && isBoxSmall[0])
          toCheck = false;
        else if ((key === 'e' || key === 'w' || key === 're' || key === 'rw') && isBoxSmall[1])
          toCheck = false;

        if (toCheck) toReturn = handle.getEntityAt(position, lowerLevel, threshold);
      }
    });

    return toReturn;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean | undefined): void {}

  getDrawable(useWebGL: boolean = false): Drawable {
    const size = vec2.mulS(this.transform.size, SceneManager.viewport.zoom);
    const isBoxSmall = [size[0] < 30, size[1] < 30];

    const ops: DrawOp[] = [
      {
        type: 'rect',
        data: [[0, 0], this.transform.size]
      },
      {
        type: 'stroke'
      },
      { type: 'beginPath' }
    ];

    Object.entries(this.m_handles).forEach(([key, handle]: [string, GenericHandle]) => {
      if (handle.handleType !== 'rotate') {
        let render = true;

        if ((key === 'n' || key === 's') && isBoxSmall[0]) render = false;
        else if ((key === 'e' || key === 'w') && isBoxSmall[1]) render = false;

        if (render) ops.push(...handle.getDrawable(useWebGL).operations);
      }
    });

    ops.push(
      ...([
        {
          type: 'stroke'
        },
        {
          type: 'fill'
        }
      ] as DrawOp[])
    );

    return {
      operations: ops
    };
  }

  getOutlineDrawable(useWebGL: boolean = false): Drawable {
    return this.getDrawable(useWebGL);
  }

  render(): void {
    Renderer.entity(this);
  }

  asObject(duplicate: boolean = false): EntityObject {
    return {} as EntityObject;
  }

  toJSON(): EntityObject {
    return this.asObject(false);
  }
}

export default Manipulator;
