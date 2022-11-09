import Element, { isElement } from '@/editor/ecs/entities/element';
import SceneManager from '@editor/scene';
import { round, vec2 } from '@math';
import CanvasStats from '../stats';
import Renderer from '../renderer';
import InputManager from '@/editor/input';
import SelectionManager from '@/editor/selection';
import ImageMedia, { isImage } from '@/editor/ecs/entities/image';
import { RectTransform } from '@/editor/ecs/components/transform';
import Debugger from '@/utils/debugger';
import AnimationManager from '@/editor/animation';
import CanvasBackend2D from './backend2d';
import { MATH_TWO_PI } from '@/utils/constants';

class Canvas2D extends CanvasBackend2D {
  private m_outlineImageQueue: ImageEntity[] = [];
  private m_outlineCircleQueue: vec2[] = [];
  private m_outlineSquareQueue: vec2[] = [];
  private m_outlineFilledSquareQueue: vec2[] = [];
  private m_outlineCurrentTransform: mat3;

  private vertexOutline(vertex: VertexEntity, selected?: boolean) {
    const position = vertex.transform.position;

    // TODO: Check if caching improves performance

    if (!selected) {
      this.m_outlineSquareQueue.push(vec2.transformMat3(position, this.m_outlineCurrentTransform));
      return;
    }

    this.m_outlineFilledSquareQueue.push(
      vec2.transformMat3(position, this.m_outlineCurrentTransform)
    );

    if (vertex.left) {
      const left = vec2.add(vertex.left.transform.position, position);
      this.m_ctx.moveTo(...left);
      this.m_ctx.lineTo(...position);
      this.m_outlineCircleQueue.push(
        vec2.transformMat3(left, this.m_outlineCurrentTransform, left)
      );
    } else this.m_ctx.moveTo(...position);

    if (vertex.right) {
      const right = vec2.add(vertex.right.transform.position, position);
      this.m_ctx.lineTo(...right);
      this.m_outlineCircleQueue.push(
        vec2.transformMat3(right, this.m_outlineCurrentTransform, right)
      );
    }
  }

  private debugging() {
    console.log('debugging');
  }

  entity(entity: Entity, options: { inheritStrokeWidth?: boolean } = {}) {
    if (isElement(entity)) {
      this.element(entity, options);
      return;
    }

    if (isImage(entity)) {
      this.image(entity);
      return;
    }

    this.m_ctx.save();

    const matrix = entity.transform.mat3;
    this.m_ctx.transform(matrix[0], matrix[3], matrix[1], matrix[4], matrix[2], matrix[5]);

    this.draw(entity.getDrawable());

    this.m_ctx.restore();
  }

  element(element: Element, options: { inheritStrokeWidth?: boolean } = {}) {
    if (!SceneManager.isVisible(element)) return;

    this.m_ctx.save();

    const matrix = element.transform.mat3;
    this.m_ctx.transform(matrix[0], matrix[3], matrix[1], matrix[4], matrix[2], matrix[5]);

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

  image(image: ImageEntity) {
    // TODO: Bilinar Filtering
    if (!SceneManager.isVisible(image)) return;

    this.m_ctx.save();

    const matrix = image.transform.mat3;
    this.m_ctx.transform(matrix[0], matrix[3], matrix[1], matrix[4], matrix[2], matrix[5]);

    this.m_ctx.drawImage(image.source, 0, 0, ...image.transform.staticSize);

    this.m_ctx.restore();
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
    this.m_ctx.transform(matrix[0], matrix[3], matrix[1], matrix[4], matrix[2], matrix[5]);

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

        const matrix = this.m_outlineImageQueue[i].transform.mat3;
        this.m_ctx.transform(matrix[0], matrix[3], matrix[1], matrix[4], matrix[2], matrix[5]);

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

  endFrame({ stats, debugging }: { stats?: Stats; debugging?: boolean }): void {
    super.endFrame({ stats });
    if (debugging) this.debugging();
  }
}

// public element(element: Element) {
//   this.m_stats.entity();
//   if (!SceneManager.isVisible(element)) return;

//   this.m_ctx.save();
//   this.m_ctx.strokeStyle = `rgba(0, 0, 0, 1.0)`;
//   this.m_ctx.lineWidth = 1;

//   const matrix = element.transform.mat3;
//   this.m_ctx.transform(matrix[0], matrix[3], matrix[1], matrix[4], matrix[2], matrix[5]);

//   this.draw(element.getDrawable(false));

//   if (element.fill && element.fill.visible) {
//     this.m_ctx.fillStyle = element.fill.color.toString();
//     this.m_ctx.fill();
//   }

//   if (element.stroke && element.stroke.visible) {
//     this.m_ctx.strokeStyle = element.stroke.color.toString();
//     this.m_ctx.lineJoin = element.stroke.corner;
//     this.m_ctx.lineWidth = element.stroke.width;
//     // TODO: inside/outside strokes
//     // this.m_ctx.clip();
//     // this.m_ctx.lineWidth *= 2;
//     this.m_ctx.stroke();
//   }

//   if (Renderer.debugging && Renderer.debug.box) {
//     if (
//       (InputManager.hover.element && InputManager.hover.element.id === element.id) ||
//       SelectionManager.has(element.id)
//     ) {
//       this.m_ctx.fillStyle = 'rgba(220, 20, 60, 0.2)';
//       this.m_ctx.strokeStyle = 'rgb(220, 20, 60)';
//     } else {
//       this.m_ctx.fillStyle = 'rgba(0, 255, 127, 0.2)';
//       this.m_ctx.strokeStyle = 'rgb(0, 255, 127)';
//     }
//     this.beginPath();
//     element.forEachBezier((b) => {
//       const box = b.boundingBox;
//       this.m_ctx.rect(box[0][0], box[0][1], box[1][0] - box[0][0], box[1][1] - box[0][1]);
//     });
//     const b = element.transform.unrotatedBoundingBox;
//     const box = b.map((p) => vec2.sub(p, element.transform.position));
//     this.m_ctx.rect(box[0][0], box[0][1], box[1][0] - box[0][0], box[1][1] - box[0][1]);
//     this.fill();
//     this.stroke();
//   }

//   this.m_ctx.restore();
//   this.m_ctx.save();

//   this.m_ctx.translate(...element.transform.position);

//   for (const point of element.points) {
//     this.m_ctx.fillStyle = 'rgb(250, 50, 50)';
//     this.beginPath();
//     this.circle({ type: 'circle', data: [point, 20] });
//     this.fill();
//     this.closePath();
//   }

//   this.m_ctx.restore();

//   if (Renderer.debugging && Renderer.debug.box) {
//     if (
//       (InputManager.hover.element && InputManager.hover.element.id === element.id) ||
//       SelectionManager.has(element.id)
//     ) {
//       this.m_ctx.fillStyle = 'rgba(220, 20, 60, 0.2)';
//       this.m_ctx.strokeStyle = 'rgb(220, 20, 60)';
//     } else {
//       this.m_ctx.fillStyle = 'rgba(0, 255, 127, 0.2)';
//       this.m_ctx.strokeStyle = 'rgb(0, 255, 127)';
//     }
//     const box = (element as Element).transform.boundingBox;
//     this.beginPath();
//     this.m_ctx.rect(box[0][0], box[0][1], box[1][0] - box[0][0], box[1][1] - box[0][1]);
//     this.fill();
//     this.stroke();
//   }
//   this.m_stats.draw();
// }

// public image(image: ImageMedia) {
//   // TODO: Bilinar Filtering
//   if (!SceneManager.isVisible(image)) return;
//   this.m_stats.draw();

//   this.m_ctx.save();

//   const matrix = image.transform.mat3;
//   this.m_ctx.transform(matrix[0], matrix[3], matrix[1], matrix[4], matrix[2], matrix[5]);
//   this.m_ctx.drawImage(image.source, 0, 0, ...image.transform.staticSize);

//   this.m_ctx.restore();

//   if (Renderer.debugging && Renderer.debug.box) {
//     if (
//       (InputManager.hover.element && InputManager.hover.element.id === image.id) ||
//       SelectionManager.has(image.id)
//     ) {
//       this.m_ctx.fillStyle = 'rgba(220, 20, 60, 0.2)';
//       this.m_ctx.strokeStyle = 'rgb(220, 20, 60)';
//     } else {
//       this.m_ctx.fillStyle = 'rgba(0, 255, 127, 0.2)';
//       this.m_ctx.strokeStyle = 'rgb(0, 255, 127)';
//     }
//     const box = image.transform.boundingBox;
//     this.beginPath();
//     this.m_ctx.rect(box[0][0], box[0][1], box[1][0] - box[0][0], box[1][1] - box[0][1]);
//     this.fill();
//     this.stroke();
//   }
// }

// public debugging() {
//   const width = 150;
//   const lAlign = 5;
//   const rAlign = 40;
//   const timers = Debugger.timers;

//   this.m_ctx.setTransform(1, 0, 0, 1, this.size[0] - width, 0);
//   this.m_ctx.fillStyle = 'rgba(0.0, 0.0, 0.0, 0.5)';
//   this.m_ctx.fillRect(0, 0, width, 170 + timers.size * 55);

//   this.m_ctx.textAlign = 'left';
//   this.m_ctx.font = 'bold 12px Helvetica,Arial,sans-serif';
//   this.m_ctx.fillStyle = 'white';
//   this.m_ctx.textBaseline = 'top';

//   this.m_ctx.fillStyle = this.m_stats.getColor(this.m_stats.fps[0]);
//   this.m_ctx.fillText('FPS:', lAlign, 5);
//   this.m_ctx.fillText(
//     `${AnimationManager.playing ? AnimationManager.fps : this.m_stats.fps[0]}  [${
//       this.m_stats.fps[1]
//     } MS]`,
//     rAlign,
//     5
//   );

//   this.m_ctx.fillStyle = this.m_stats.getColor(this.m_stats.avg[0]);
//   this.m_ctx.fillText('AVG:', lAlign, 30);
//   this.m_ctx.fillText(`${this.m_stats.avg[0]}  [${this.m_stats.avg[1]} MS]`, rAlign, 30);

//   this.m_ctx.fillStyle = this.m_stats.getColor(this.m_stats.min[0]);
//   this.m_ctx.fillText('MIN:', lAlign, 55);
//   this.m_ctx.fillText(`${this.m_stats.min[0]}  [${this.m_stats.min[1]} MS]`, rAlign, 55);

//   this.m_ctx.fillStyle = 'mediumorchid';
//   this.m_ctx.fillText('MEM:', lAlign, 80);
//   this.m_ctx.fillText(`${this.m_stats.mem} / ${this.m_stats.heap} MB`, rAlign, 80);

//   this.m_ctx.fillStyle = 'white';
//   this.m_ctx.fillText('BOX:', lAlign, 105);
//   this.m_ctx.fillText(`${Renderer.debug.box.toString().toUpperCase()}`, rAlign, 105);

//   this.m_ctx.fillText('ENT:', lAlign, 130);
//   this.m_ctx.fillText(`${this.m_stats.entities}`, rAlign, 130);

//   this.m_ctx.fillText('DRW:', lAlign, 155);
//   this.m_ctx.fillText(`${this.m_stats.drawn}`, rAlign, 155);

//   let i = 0;

//   timers.forEach((timer, id) => {
//     this.m_ctx.fillText(id + ':', lAlign, 180 + 55 * i);
//     this.m_ctx.fillText(`${timer.value.toFixed(4)}ms`, rAlign + 10, 180 + 55 * i);
//     this.m_ctx.fillText(`${timer.entries}`, rAlign + 10, 195 + 55 * i);
//     this.m_ctx.fillText(
//       `${(timer.value / timer.entries).toFixed(4)}ms`,
//       rAlign + 10,
//       210 + 55 * i
//     );
//     ++i;
//   });
// }

export default Canvas2D;
