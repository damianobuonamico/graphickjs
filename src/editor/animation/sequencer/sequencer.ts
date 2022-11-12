import InputManager from '@/editor/input';
import { clamp, isPointInBox, map, round, vec2 } from '@/math';
import { ZOOM_STEP } from '@/utils/constants';
import { BUTTONS } from '@/utils/keys';
import { fillObject } from '@/utils/utils';
import CanvasBackend2D from '../../renderer/2D/backend2d';
import Sequence from './sequence';

class Sequencer extends CanvasBackend2D {
  private m_shouldUpdate = false;
  private m_sequence = new Sequence();
  private m_lastHover: SequenceNode | undefined = undefined;
  private m_hover: SequenceNode | undefined = undefined;

  private m_viewport: ViewportState;

  private m_minZoom = 0.5;
  private m_maxZoom = 2;

  private m_local: PointerCoord;

  constructor(canvas: HTMLCanvasElement) {
    super();

    this.m_viewport = {
      position: [0, 0],
      zoom: 1,
      rotation: 0
    };

    this.m_local = fillObject(
      {},
      {
        position: vec2.create(),
        movement: vec2.create(),
        delta: vec2.create(),
        origin: vec2.create()
      }
    );

    this.setup(canvas);
  }

  private set zoom(value: number | [number, vec2]) {
    const isArray = Array.isArray(value);
    const zoom = round(clamp(isArray ? value[0] : value, this.m_minZoom, this.m_maxZoom), 4);

    if (isArray) {
      const delta = vec2.sub(
        this.clientToLocal(vec2.clone(value[1]), { zoom }),
        this.clientToLocal(vec2.clone(value[1]))
      );

      this.m_viewport.position = vec2.add(this.m_viewport.position, delta);
    }

    this.m_viewport.zoom = zoom;
  }

  private clientToLocal(position: vec2, override: Partial<ViewportState> = {}) {
    const viewport = fillObject<ViewportState>(override, this.m_viewport);
    const local = vec2.create();

    vec2.sub(
      vec2.divS(vec2.sub(position, this.offset, local), viewport.zoom, local),
      viewport.position,
      local
    );

    return local;
  }

  private renderNode(node: SequenceNode) {
    this.m_ctx.save();

    const padding = 10;

    this.m_ctx.shadowColor = 'rgba(0, 0, 0, 0.4)';
    this.m_ctx.shadowBlur = 10 * this.m_viewport.zoom;
    this.m_ctx.shadowOffsetY = 2 * this.m_viewport.zoom;
    this.m_ctx.fillStyle = node.color;

    const width = this.m_ctx.measureText(node.name).width;
    const height = 12 + 2 * padding;

    node.size = [width + 2 * padding, height];

    this.roundedRect(node.position, node.size, 5);
    this.fill();

    this.m_ctx.restore();
    this.m_ctx.fillStyle = 'white';

    this.m_ctx.fillText(node.name, node.position[0] + padding, node.position[1] + height / 2 + 1);
  }

  private renderFn() {
    this.beginFrame({
      color: '#0E1117',
      position: this.m_viewport.position,
      zoom: this.m_viewport.zoom
    });

    this.m_ctx.font = '12px system-ui, sans-serif';
    this.m_ctx.textBaseline = 'middle';

    this.m_sequence.forEach(this.renderNode.bind(this));

    if (this.m_hover) {
      this.m_ctx.strokeStyle = 'rgba(56, 195, 242, 1.0)';
      this.m_ctx.lineWidth = 1.5 / this.m_viewport.zoom;

      this.roundedRect(this.m_hover.position, this.m_hover.size, 5);
      this.stroke();
    }

    this.endFrame({});
  }

  resize() {
    super.resize();

    this.m_shouldUpdate = true;
    this.render();
  }

  onPointerDown() {
    this.m_local.movement = vec2.create();
    this.m_local.position = this.clientToLocal(InputManager.client.position);
    this.m_local.delta = vec2.create();
    this.m_local.origin = vec2.clone(this.m_local.position);
  }

  onPointerMove() {
    this.m_local.movement = vec2.divS(InputManager.client.movement, this.m_viewport.zoom);
    this.m_local.position = this.clientToLocal(InputManager.client.position);
    this.m_local.delta = vec2.sub(this.m_local.position, this.m_local.origin);

    if (!InputManager.down) {
      let hover: SequenceNode | undefined = undefined;

      this.m_sequence.forEach((node) => {
        if (hover) return;

        if (isPointInBox(this.m_local.position, node.boundingBox)) hover = node;
      });

      this.m_lastHover = this.m_hover;
      this.m_hover = hover;

      if (this.m_hover !== this.m_lastHover) this.render();

      return;
    }

    if (InputManager.button === BUTTONS.MIDDLE) {
      vec2.add(this.m_viewport.position, this.m_local.movement, this.m_viewport.position);
    } else if (InputManager.keys.space) {
      if (InputManager.keys.ctrl) {
        const movement =
          Math.abs(InputManager.client.movement[0]) > Math.abs(InputManager.client.movement[1])
            ? InputManager.client.movement[0]
            : -InputManager.client.movement[1];

        this.zoom = [
          this.m_viewport.zoom * (1 + (movement * ZOOM_STEP) / 500),
          InputManager.client.origin
        ];
      } else {
        vec2.add(this.m_viewport.position, this.m_local.movement, this.m_viewport.position);
      }
    } else if (InputManager.button === BUTTONS.RIGHT) {
    } else {
      if (this.m_hover)
        vec2.add(this.m_hover.position, this.m_local.movement, this.m_hover.position);
    }

    this.render();
  }

  onPointerUp() {}

  onWheel(e: WheelEvent) {
    if (!InputManager.keys.ctrl) return;

    this.zoom = [
      map(-e.deltaY, -100, 100, 1 - ZOOM_STEP / 10, 1 + ZOOM_STEP / 10) * this.m_viewport.zoom,
      InputManager.client.position
    ];

    this.render();
  }

  render() {
    // if (!this.m_shouldUpdate) return;
    requestAnimationFrame(this.renderFn.bind(this));
    // this.m_shouldUpdate = false;
  }

  animate(fps: number) {
    this.renderFn();
    this.m_sequence.animate(fps);
  }

  add(entity: Entity) {
    this.m_sequence.add(entity);
    this.render();
  }

  toJSON() {
    return this.m_sequence.toJSON();
  }

  load(sequence: Entity[]) {
    this.m_sequence.load(sequence);
    this.render();
  }
}

export default Sequencer;
