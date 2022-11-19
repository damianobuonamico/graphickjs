import Element, { isElement } from '@/editor/ecs/entities/element';
import SceneManager from '@editor/scene';
import { round, vec2 } from '@math';
import InputManager from '@/editor/input';
import ImageMedia, { isImage } from '@/editor/ecs/entities/image';
import CanvasBackend2D from './backend2d';
import { MATH_TWO_PI } from '@/utils/constants';
import Renderer from '../renderer';
import SelectionManager from '@/editor/selection';
import { isCompleteTransform } from '@/editor/ecs/components/transform';

class Canvas2D extends CanvasBackend2D {
  private m_debuggerBinded: boolean = false;
  private m_debuggerEntities: Map<string, Entity> = new Map();

  private m_outlineImageQueue: ImageEntity[] = [];
  private m_outlineCircleQueue: vec2[] = [];
  private m_outlineSquareQueue: vec2[] = [];
  private m_outlineFilledSquareQueue: vec2[] = [];
  private m_outlineCurrentTransform: mat3;

  constructor() {
    super();
  }

  private vertexOutline(vertex: VertexEntity, selected?: boolean) {
    const position = vertex.transform.position.value;

    // TODO: Check if caching improves performance

    if (!selected) {
      this.m_outlineSquareQueue.push(vec2.transformMat3(position, this.m_outlineCurrentTransform));
      return;
    }

    this.m_outlineFilledSquareQueue.push(
      vec2.transformMat3(position, this.m_outlineCurrentTransform)
    );

    const left = vertex.transform.left?.value;
    const right = vertex.transform.right?.value;

    if (left) {
      vec2.add(left, position, left);
      this.m_ctx.moveTo(...left);
      this.m_ctx.lineTo(...position);
      this.m_outlineCircleQueue.push(
        vec2.transformMat3(left, this.m_outlineCurrentTransform, left)
      );
    } else this.m_ctx.moveTo(...position);

    if (right) {
      vec2.add(right, position, right);
      this.m_ctx.lineTo(...right);
      this.m_outlineCircleQueue.push(
        vec2.transformMat3(right, this.m_outlineCurrentTransform, right)
      );
    }
  }

  private drawMonitor(
    value: string,
    values: number[],
    max: number,
    background: string,
    color: string,
    width: number,
    padding: number
  ) {
    const height = 40;

    this.m_ctx.save();
    this.m_ctx.globalAlpha = 0.8;
    this.m_ctx.fillStyle = background;
    this.m_ctx.fillRect(0, 0, width + padding * 2, height + 18);

    this.m_ctx.translate(padding, padding + 1);
    this.m_ctx.fillStyle = color;
    this.m_ctx.globalAlpha = 0.1;
    this.m_ctx.fillRect(0, 11, width, height);

    this.m_ctx.globalAlpha = 1;
    this.m_ctx.fillText(value, 0, 0);

    this.m_ctx.translate(width, height + 11);
    this.m_ctx.scale(-1, -1);
    this.beginPath();

    const multiplier = height / max;

    for (let i = 0, n = values.length; i < n; i++) {
      this.m_ctx.rect(n - i, 0, 1, Math.min(values[i], max) * multiplier);
    }

    this.fill();

    this.m_ctx.restore();

    return height + 18;
  }

  private drawDebuggingProperty(name: string, value: string, width: number) {
    this.m_ctx.textAlign = 'right';
    this.m_ctx.fillText(value, width, 0);

    this.m_ctx.textAlign = 'left';
    if (name.length) this.m_ctx.fillText(name + ':', 0, 0);

    return 10;
  }

  private debugging(
    stats?: RendererStats,
    { entityBox, segmentBox, vertices, opacity }: Partial<DebugState> = {}
  ) {
    this.m_ctx.save();

    this.m_ctx.globalAlpha = Math.max(opacity || 1, 0.1);
    this.m_ctx.lineWidth = 1.5 / SceneManager.viewport.zoom;

    this.m_debuggerEntities.forEach((entity) => {
      if (
        (InputManager.hover.element && InputManager.hover.element.id === entity.id) ||
        SelectionManager.has(entity.id)
      ) {
        this.m_ctx.fillStyle = 'rgba(220, 20, 60, 0.1)';
        this.m_ctx.strokeStyle = 'rgba(220, 20, 60, 0.5)';
      } else {
        this.m_ctx.fillStyle = 'rgba(0, 255, 127, 0.1)';
        this.m_ctx.strokeStyle = 'rgba(0, 255, 127, 0.5)';
      }

      this.beginPath();

      if (entityBox && isCompleteTransform(entity.transform)) {
        const box = entity.transform.boundingBox;
        this.rect(box[0], vec2.sub(box[1], box[0]));
      }

      if (segmentBox && isElement(entity)) {
        this.m_ctx.save();

        this.transform(entity.transform.mat3);

        entity.forEachBezier((b) => {
          const box = b.boundingBox;
          this.m_ctx.rect(box[0][0], box[0][1], box[1][0] - box[0][0], box[1][1] - box[0][1]);
        });

        this.m_ctx.restore();
      }

      this.fill();
      this.stroke();

      if (vertices && isElement(entity)) {
        entity.forEach((vertex) =>
          this.debugCircle({
            position: entity.transform.transform(vertex.transform.position.value),
            radius: 4,
            color:
              InputManager.hover.entity === vertex.position || entity.selection.has(vertex.id)
                ? 'rgba(220, 20, 60, 0.5)'
                : 'rgba(0, 255, 127, 0.5)'
          })
        );
      }
    });

    if (!stats) return;

    const WIDTH = 100;
    const PADDING = 3;

    this.m_ctx.setTransform(
      this.m_dpr,
      0,
      0,
      this.m_dpr,
      this.m_canvas.width - (WIDTH + PADDING * 2) * this.m_dpr,
      0
    );

    this.m_ctx.font = 'bold ' + 9 + 'px Helvetica,Arial,sans-serif';
    this.m_ctx.textBaseline = 'top';

    this.m_ctx.translate(
      0,
      this.drawMonitor(
        `${Math.round(stats.fps[stats.fps.length - 1] || 0)} FPS (${stats.minFps}-${stats.maxFps})`,
        stats.fps,
        100,
        '#002',
        '#0FF',
        WIDTH,
        PADDING
      )
    );

    this.m_ctx.translate(
      0,
      this.drawMonitor(
        `${Math.round(stats.ms[stats.ms.length - 1] || 0)} MS (${stats.minMs}-${stats.maxMs})`,
        stats.ms,
        30,
        '#020',
        '#0F0',
        WIDTH,
        PADDING
      )
    );

    if (stats.hasMemoryStats) {
      this.m_ctx.translate(
        0,
        this.drawMonitor(
          `${Math.round(stats.memory[stats.memory.length - 1] || 0)} MB (${stats.minMemory}-${
            stats.maxMemory
          } / ${stats.availableMemory})`,
          stats.memory,
          stats.availableMemory,
          '#201',
          '#F08',
          WIDTH,
          PADDING
        )
      );
    }

    // TODO: Draw checkboxes

    const properties: [string, string][] = [
      ['ENTITY BOX', (entityBox || false).toString().toUpperCase()],
      ['SEGMENT BOX', (segmentBox || false).toString().toUpperCase()],
      ['ENTITIES', this.m_debuggerEntities.size.toString()],
      ['', ''],
      ['POS X', round(SceneManager.viewport.position[0], 2).toString()],
      ['POS Y', round(SceneManager.viewport.position[1], 2).toString()],
      ['ZOOM', round(SceneManager.viewport.zoom, 2).toString()],
      ['', ''],
      ['MOUSE X', round(InputManager.scene.position[0], 2).toString()],
      ['MOUSE Y', round(InputManager.scene.position[1], 2).toString()]
    ];

    this.m_ctx.globalAlpha = 0.8;
    this.m_ctx.fillStyle = '#0E1117';
    this.m_ctx.fillRect(0, 0, WIDTH + PADDING * 2, properties.length * 10 + PADDING);

    this.m_ctx.globalAlpha = 1;
    this.m_ctx.translate(PADDING, PADDING);
    this.m_ctx.fillStyle = '#FFF';

    for (const property of properties) {
      this.m_ctx.translate(0, this.drawDebuggingProperty(property[0], property[1], WIDTH));
    }

    this.m_ctx.restore();
  }

  private bindDebugger(unbind?: boolean) {
    if (unbind) {
      this.entity = this.drawEntity;
      this.element = this.drawElement;
      this.image = this.drawImage;

      this.m_debuggerBinded = false;
    } else {
      this.entity = this.drawEntityDebugged;
      this.element = this.drawElementDebugged;
      this.image = this.drawImageDebugged;

      this.m_debuggerBinded = true;
    }

    Renderer.refresh();
  }

  private drawEntity(entity: Entity, options: { inheritStrokeWidth?: boolean } = {}) {
    if (isElement(entity)) {
      this.element(entity, options);
      return;
    }

    if (isImage(entity)) {
      this.image(entity);
      return;
    }

    if (!SceneManager.isVisible(entity)) return false;

    this.m_ctx.save();

    if (isCompleteTransform(entity.transform)) this.transform(entity.transform.mat3);
    this.draw(entity.getDrawable());

    this.m_ctx.restore();
  }

  private drawElement(element: Element, options: { inheritStrokeWidth?: boolean } = {}) {
    if (!SceneManager.isVisible(element)) return false;

    this.m_ctx.save();

    this.m_ctx.globalAlpha = element.opacity;
    this.transform(element.transform.mat3);
    this.beginPath();
    this.draw(element.getDrawable());

    if (element.fill && element.fill.visible) {
      this.m_ctx.fillStyle = element.fill.color.toString();
      this.fill();
    }

    if (element.stroke && element.stroke.visible) {
      this.m_ctx.strokeStyle = element.stroke.color.toString();
      this.m_ctx.lineJoin = element.stroke.corner;
      if (!options.inheritStrokeWidth) this.m_ctx.lineWidth = element.stroke.width;
      if (Array.isArray(element.stroke.style)) this.m_ctx.setLineDash(element.stroke.style);

      // TODO: inside/outside strokes
      // this.m_ctx.clip();
      // this.m_ctx.lineWidth *= 2;

      this.stroke();
    }

    this.m_ctx.restore();
  }

  private drawImage(image: ImageEntity) {
    // TODO: Bilinar Filtering / Lanczos Resampling
    if (!SceneManager.isVisible(image)) return false;

    this.m_ctx.save();

    this.transform(image.transform.mat3);
    this.m_ctx.drawImage(image.source, 0, 0, ...image.transform.staticSize);

    this.m_ctx.restore();
  }

  private drawEntityDebugged(entity: Entity, options: { inheritStrokeWidth?: boolean }) {
    if (this.drawEntity(entity, options) === false) return;

    this.m_debuggerEntities.set(entity.id, entity);
  }

  private drawElementDebugged(element: Element, options: { inheritStrokeWidth?: boolean }) {
    if (this.drawElement(element, options) === false) return;

    this.m_debuggerEntities.set(element.id, element);
  }

  private drawImageDebugged(image: ImageEntity) {
    if (this.drawImage(image) === false) return;

    this.m_debuggerEntities.set(image.id, image);
  }

  entity: (entity: Entity, options: { inheritStrokeWidth?: boolean }) => void = this.drawEntity;

  element: (element: Element, options: { inheritStrokeWidth?: boolean }) => void = this.drawElement;

  image: (image: ImageEntity) => void = this.drawImage;

  debugCircle(options: { position: vec2; radius?: number; color?: string }) {
    super.debugCircle({ ...options, radius: (options.radius || 5) / SceneManager.viewport.zoom });
  }

  debugRect(options: { position: vec2; size?: vec2 | number; centered?: boolean; color?: string }) {
    super.debugRect({
      ...options,
      size: vec2.divS(
        typeof options.size === 'number' ? [options.size, options.size] : options.size || [10, 10],
        SceneManager.viewport.zoom
      )
    });
  }

  beginOutline() {
    this.m_ctx.strokeStyle = 'rgba(56, 195, 242, 1.0)';
    this.m_ctx.fillStyle = 'rgba(56, 195, 242, 1.0)';
    this.m_ctx.lineWidth = 1.5 / SceneManager.viewport.zoom;

    this.beginPath();
  }

  outline(entity: Element | ImageMedia, skipVertices: boolean = false) {
    if (!SceneManager.isVisible(entity)) return;

    if (isImage(entity)) {
      this.m_outlineImageQueue.push(entity);
      return;
    }

    this.m_ctx.save();

    const matrix = entity.transform.mat3;
    this.transform(matrix);
    this.draw(entity.getOutlineDrawable());

    if (!skipVertices && InputManager.tool.isVertex && isElement(entity)) {
      this.m_outlineCurrentTransform = matrix;
      entity.forEach(this.vertexOutline.bind(this));
    }

    this.m_ctx.restore();
  }

  endOutline() {
    this.stroke();

    if (this.m_outlineImageQueue.length) {
      this.beginPath();

      for (let i = 0, n = this.m_outlineImageQueue.length; i < n; i++) {
        this.m_ctx.save();

        this.transform(this.m_outlineImageQueue[i].transform.mat3);
        this.draw(this.m_outlineImageQueue[i].getOutlineDrawable());

        this.m_ctx.restore();
      }

      this.stroke();

      this.m_outlineImageQueue = [];
    }

    let radius = 3 / SceneManager.viewport.zoom;
    let size = 2 * radius;

    if (this.m_outlineCircleQueue.length || this.m_outlineFilledSquareQueue.length) {
      this.beginPath();

      for (let i = 0, n = this.m_outlineCircleQueue.length; i < n; i++) {
        const position = this.m_outlineCircleQueue[i];

        this.m_ctx.moveTo(position[0] + radius, position[1]);
        this.m_ctx.arc(...position, radius, 0, MATH_TWO_PI);
      }

      radius = 2.5 / SceneManager.viewport.zoom;
      size = 2 * radius;

      for (let i = 0, n = this.m_outlineFilledSquareQueue.length; i < n; i++) {
        const position = this.m_outlineFilledSquareQueue[i];

        this.m_ctx.rect(position[0] - radius, position[1] - radius, size, size);
      }

      this.fill();

      this.m_outlineCircleQueue = [];
      this.m_outlineFilledSquareQueue = [];
    }

    if (this.m_outlineSquareQueue.length) {
      radius = 2 / SceneManager.viewport.zoom;
      size = 2 * radius;

      this.m_ctx.fillStyle = '#FFFFFF';
      this.m_ctx.lineWidth = 1 / SceneManager.viewport.zoom;

      this.beginPath();

      for (let i = 0, n = this.m_outlineSquareQueue.length; i < n; i++) {
        const position = this.m_outlineSquareQueue[i];

        this.m_ctx.rect(position[0] - radius, position[1] - radius, size, size);
      }

      this.fill();
      this.stroke();

      this.m_outlineSquareQueue = [];
    }
  }

  beginFrame(options: {
    color?: string;
    zoom?: number;
    position?: vec2;
    stats?: RendererStats;
    debugging?: boolean;
  }): void {
    super.beginFrame({ ...options, stats: options.debugging ? options.stats : undefined });

    if (this.m_debuggerBinded !== options.debugging) this.bindDebugger(!options.debugging);
  }

  endFrame({
    stats,
    debugging,
    debug
  }: {
    stats?: RendererStats;
    debugging?: boolean;
    debug?: DebugState;
  }): void {
    super.endFrame(debugging ? { stats } : {});

    if (debugging) this.debugging(stats, debug);
    this.m_debuggerEntities.clear();
  }
}

export default Canvas2D;
