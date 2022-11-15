import { Vec2Value } from '@/editor/history/value';
import InputManager from '@/editor/input';
import {
  clamp,
  closestPointToBox,
  doesLineIntersectLine,
  isPointInBox,
  map,
  round,
  vec2
} from '@/math';
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
  private m_lastLargeHover: SequenceNode | undefined = undefined;
  private m_largeHover: SequenceNode | undefined = undefined;

  private m_targetHover: SequenceNode | undefined = undefined;
  private m_cutPosition: vec2 | undefined = undefined;

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
    // this.m_ctx.fillStyle = node.color;
    this.m_ctx.fillStyle = 'green';

    // const width = this.m_ctx.measureText(node.name).width;
    const width = this.m_ctx.measureText(node.id).width;
    const height = 12 + 2 * padding;

    node.size = [width + 2 * padding, height];

    const position = node.position.value;

    this.roundedRect(position, node.size, 5);
    this.fill();

    this.m_ctx.fillStyle = 'rgba(255, 255, 255, 0.2)';
    this.roundedRect(position, [node.size[0] * node.percent, node.size[1]], 5);
    this.fill();

    this.m_ctx.restore();
    this.m_ctx.fillStyle = 'white';

    this.m_ctx.fillText(node.id, position[0] + padding, position[1] + height / 2 + 1);
    // this.m_ctx.fillText(node.name, node.position[0] + padding, node.position[1] + height / 2 + 1);
  }

  private renderFn() {
    this.beginFrame({
      color: '#0E1117',
      position: this.m_viewport.position,
      zoom: this.m_viewport.zoom
    });

    this.m_ctx.font = '12px system-ui, sans-serif';
    this.m_ctx.textBaseline = 'middle';

    this.beginPath();
    this.m_ctx.strokeStyle = 'rgba(56, 195, 242, 1.0)';
    this.m_ctx.lineWidth = 2 / this.m_viewport.zoom;

    this.m_sequence.forEach((node) => {
      const box = node.boundingBox;
      const center = vec2.mid(box[0], box[1]);

      node.forEach((link) => {
        const targetBox = link.boundingBox;
        const target = vec2.mid(targetBox[0], targetBox[1]);

        this.moveTo(center);
        this.lineTo(target);

        const mid = vec2.mid(target, center);

        const direction = vec2.mulS(
          vec2.normalize(vec2.sub(target, center)),
          8 / this.m_viewport.zoom
        );
        const a1 = vec2.add(vec2.rotate(direction, [0, 0], (3 * Math.PI) / 4), mid);
        const a2 = vec2.add(vec2.rotate(direction, [0, 0], -(3 * Math.PI) / 4), mid);

        this.moveTo(a1);
        this.lineTo(mid);
        this.lineTo(a2);
      });
    });

    this.stroke();

    if (this.m_largeHover && !this.m_cutPosition) {
      const radius = 4 / this.m_viewport.zoom;
      const box = this.m_largeHover.boundingBox;

      if (InputManager.down) {
        this.beginPath();
        this.m_ctx.fillStyle = 'rgba(56, 195, 242, 1.0)';
        this.m_ctx.strokeStyle = 'rgba(56, 195, 242, 1.0)';
        this.m_ctx.lineWidth = 2 / this.m_viewport.zoom;

        this.moveTo(vec2.mid(box[0], box[1]));

        if (this.m_targetHover && this.m_targetHover.id !== this.m_largeHover.id) {
          const target = this.m_targetHover.boundingBox;
          this.lineTo(vec2.mid(target[0], target[1]));
          this.stroke();
        } else {
          this.lineTo(this.m_local.position);
          this.stroke();

          this.beginPath();
          this.circle(this.m_local.position, radius);
          this.fill();
        }
      } else {
        this.beginPath();
        this.m_ctx.fillStyle = '#575c62';

        vec2.subS(box[0], 2 * radius, box[0]);
        vec2.addS(box[1], 2 * radius, box[1]);

        this.circle(closestPointToBox(this.m_local.position, box)[0], radius);
        this.fill();
      }
    }

    this.m_sequence.forEach(this.renderNode.bind(this));

    if (this.m_hover) {
      this.m_ctx.strokeStyle = 'rgba(56, 195, 242, 1.0)';
      this.m_ctx.lineWidth = 1.5 / this.m_viewport.zoom;

      this.roundedRect(this.m_hover.position.value, this.m_hover.size, 5);
      this.stroke();
    }

    if (this.m_cutPosition) {
      this.m_ctx.strokeStyle = '#575c62';
      // this.m_ctx.setLineDash([3 / this.m_viewport.zoom]);
      this.beginPath();
      this.moveTo(this.m_local.origin);
      this.lineTo(this.m_cutPosition);
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

    // TODO: Bind listeners
    if (InputManager.button === BUTTONS.LEFT) {
      this.onPointerMove = this.onPointerDrag;

      if (InputManager.keys.space) {
        if (InputManager.keys.ctrl) this.onPointerMove = this.onPointerZoom;
        else this.onPointerMove = this.onPointerPan;
      }
    } else if (InputManager.button === BUTTONS.RIGHT) this.onPointerMove = this.onPointerCut;
    else if (InputManager.button === BUTTONS.MIDDLE) this.onPointerMove = this.onPointerPan;

    this.render();
  }

  private updatePointer() {
    this.m_local.movement = vec2.divS(InputManager.client.movement, this.m_viewport.zoom);
    this.m_local.position = this.clientToLocal(InputManager.client.position);
    this.m_local.delta = vec2.sub(this.m_local.position, this.m_local.origin);
  }

  private onPointerHover() {
    this.updatePointer();

    let hover: SequenceNode | undefined = undefined;
    let largeHover: SequenceNode | undefined = undefined;

    this.m_sequence.forEachReversed((node) => {
      if (hover) return;

      if (isPointInBox(this.m_local.position, node.boundingBox)) hover = node;
      if (isPointInBox(this.m_local.position, node.boundingBox, 20 / this.m_viewport.zoom))
        largeHover = node;
    });

    this.m_targetHover = undefined;
    this.m_lastHover = this.m_hover;
    this.m_lastLargeHover = this.m_largeHover;
    this.m_hover = hover;

    if (hover) this.m_largeHover = undefined;
    else this.m_largeHover = largeHover;

    if (
      this.m_hover !== this.m_lastHover ||
      this.m_largeHover ||
      this.m_largeHover !== this.m_lastLargeHover
    )
      this.render();
  }

  private onPointerDrag() {
    this.updatePointer();

    if (InputManager.down) {
      let targetHover: SequenceNode | undefined = undefined;

      this.m_sequence.forEachReversed((node) => {
        if (targetHover) return;

        if (isPointInBox(this.m_local.position, node.boundingBox, 20 / this.m_viewport.zoom))
          targetHover = node;
      });

      this.m_targetHover = targetHover;
    }

    if (this.m_hover) {
      this.m_hover.position.add(this.m_local.movement);
      // this.m_hover.position.value = vec2.add(this.m_hover.position.value, this.m_local.movement);
      // vec2.add(this.m_hover.position, this.m_local.movement, this.m_hover.position);
    }

    this.render();
  }

  private onPointerCut() {
    this.updatePointer();
    this.m_cutPosition = vec2.clone(this.m_local.position);

    this.render();
  }

  private onPointerPan() {
    this.updatePointer();

    vec2.add(this.m_viewport.position, this.m_local.movement, this.m_viewport.position);

    this.m_hover = undefined;
    this.m_lastHover = undefined;
    this.m_largeHover = undefined;
    this.m_lastLargeHover = undefined;
    this.m_targetHover = undefined;

    this.render();
  }

  private onPointerZoom() {
    this.updatePointer();

    const movement =
      Math.abs(InputManager.client.movement[0]) > Math.abs(InputManager.client.movement[1])
        ? InputManager.client.movement[0]
        : -InputManager.client.movement[1];

    this.zoom = [
      this.m_viewport.zoom * (1 + (movement * ZOOM_STEP) / 500),
      InputManager.client.origin
    ];

    this.m_hover = undefined;
    this.m_lastHover = undefined;
    this.m_largeHover = undefined;
    this.m_lastLargeHover = undefined;
    this.m_targetHover = undefined;

    this.render();
  }

  onPointerMove = this.onPointerHover;

  onPointerUp() {
    if (this.m_cutPosition) {
      const line: Box = [this.m_local.origin, this.m_cutPosition];

      this.m_sequence.forEach((node) => {
        const box = node.boundingBox;
        const mid = vec2.mid(box[0], box[1]);

        node.forEach((link) => {
          const target = link.boundingBox;
          if (doesLineIntersectLine(line, [vec2.mid(target[0], target[1]), mid]))
            node.unlink(link.id);
          // check intersection
        });
      });
    } else if (
      this.m_largeHover &&
      this.m_targetHover &&
      this.m_targetHover.id !== this.m_largeHover.id
    ) {
      this.m_largeHover.link(this.m_targetHover);
    }

    this.onPointerHover();

    this.onPointerMove = this.onPointerHover;
    this.m_cutPosition = undefined;

    this.render();
  }

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
    this.m_sequence.animate(fps);
    this.renderFn();
  }

  stop() {
    this.m_sequence.stop();
    this.renderFn();
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
