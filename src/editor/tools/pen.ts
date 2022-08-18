import { vec2 } from '@math';
import Bezier from '../ecs/bezier';
import Element from '../ecs/element';
import Handle from '../ecs/handle';
import Vertex from '../ecs/vertex';
import InputManager from '../input';
import SceneManager from '../scene';
import SelectionManager from '../selection';

interface PenToolData {
  vertex?: Vertex;
  element?: Element;
  overlay?: Element;
  overlayLastVertex?: Vertex;
  overlayVertex?: Vertex;
}

const onPenPointerDown = () => {
  const pen = InputManager.tool.data as PenToolData;
  const entity = InputManager.hover.entity;
  const element = InputManager.hover.element;
  const bezier = entity && entity.type === 'bezier' ? (entity as Bezier) : undefined;
  const handle = entity && entity.type === 'handle' ? (entity as Handle) : undefined;
  const vertex = handle && handle.handleType === 'vertex' ? (handle.parent as Vertex) : undefined;

  let penState = 'new' as PenState;
  if (vertex && element) {
    if (element.isOpenEnd(vertex.id)) {
      if (pen.element && element.id === pen.element.id) {
        if (pen.vertex && vertex.id === pen.vertex.id) penState = 'angle';
        else penState = 'close';
      } else {
        if (pen.element) penState = 'join';
        else penState = 'start';
      }
    } else if (SelectionManager.has(element.id)) penState = 'sub';
  } else if (bezier && element && SelectionManager.has(element.id)) penState = 'add';

  if (pen.overlay) {
    SceneManager.popRenderOverlay(pen.overlay.id);
    pen.overlay = undefined;
    pen.overlayLastVertex = undefined;
    pen.overlayVertex = undefined;
  }

  switch (penState) {
    case 'join': {
      if (!element!.isFirstVertex(vertex!.id)) element!.reverseCurves();
      pen.element!.concat(element!);
      pen.vertex = vertex!;
      SelectionManager.clear();
      SelectionManager.select(pen.element!);
      break;
    }
    case 'close': {
      pen.element!.close();
      pen.element!.generateCurves();
      vertex!.setLeft(null);
      pen.vertex = vertex;
      SelectionManager.clear();
      SelectionManager.select(pen.element!);
      break;
    }
    case 'sub': {
      break;
    }
    case 'add': {
      const vertex = element!.splitCurve(bezier!, InputManager.scene.position);
      if (vertex) {
        pen.element = element;
        pen.vertex = vertex;
      }
      break;
    }
    case 'angle': {
      pen.vertex = vertex!;
      vertex!.setRight(null);
      break;
    }
    case 'start': {
      pen.element = element!;
      pen.vertex = vertex!;
      if (element!.isFirstVertex(vertex!.id)) element!.reverseCurves();
      SelectionManager.clear();
      SelectionManager.select(element!);
      vertex!.setRight(null);
      break;
    }
    case 'new': {
      const v = new Vertex({
        position: pen.element
          ? vec2.sub(InputManager.scene.position, pen.element.position)
          : vec2.create()
      });
      pen.vertex = v;
      if (!pen.element) {
        pen.element = new Element({
          position: InputManager.scene.position
        });
        SceneManager.add(pen.element);
      }
      pen.element.push(v);
      SelectionManager.clear();
      SelectionManager.select(pen.element);
    }
  }

  function setRight(position?: vec2) {
    if (!pen.vertex) return;
    if (!pen.vertex.right) pen.vertex.setRight(position || vec2.create());
    else if (position) pen.vertex.right!.position = position;
  }

  function setLeft(position?: vec2) {
    if (!pen.vertex) return;
    if (!pen.vertex.left) pen.vertex.setLeft(position || vec2.create());
    else if (position) pen.vertex.left!.position = position;
  }

  const left = !!(pen.vertex && pen.vertex.left);
  const right = !!(pen.vertex && pen.vertex.right);

  function onPointerMove() {
    switch (penState) {
      case 'sub':
        break;
      case 'add': {
        setRight(InputManager.scene.delta);
        if (!InputManager.keys.alt) setLeft(vec2.neg(InputManager.scene.delta));
        pen.element!.recalculate();
        break;
      }
      case 'close':
      case 'join': {
        setLeft(vec2.neg(InputManager.scene.delta));
        if (!InputManager.keys.alt && right) {
          const direction = vec2.unit(InputManager.scene.delta);
          if (!vec2.equals(direction, [0, 0])) {
            setRight(vec2.mul(direction, vec2.len(pen.vertex!.right?.position!)));
          }
        }
        pen.element!.recalculate();
        break;
      }
      case 'start':
      case 'angle': {
        setRight(InputManager.scene.delta);
        if (!InputManager.keys.alt && left) {
          const direction = vec2.unit(vec2.neg(InputManager.scene.delta));
          if (!vec2.equals(direction, [0, 0])) {
            setLeft(vec2.mul(direction, vec2.len(pen.vertex!.left?.position!)));
          }
        }
        pen.element!.recalculate();
        break;
      }
      case 'new': {
        if (!InputManager.keys.alt) setLeft(vec2.neg(InputManager.scene.delta));
        setRight(InputManager.scene.delta);
        pen.element!.recalculate();
      }
    }
  }

  function onPointerUp() {
    switch (penState) {
      case 'close':
      case 'join':
      case 'add': {
        pen.element = undefined;
        pen.vertex = undefined;
        break;
      }
      case 'sub': {
        if (vec2.len(InputManager.client.delta) < 10 / SceneManager.viewport.zoom)
          element!.delete(vertex!, true);
        pen.element = undefined;
        pen.vertex = undefined;
        break;
      }
      case 'angle':
      case 'start':
      case 'new':
    }
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

export function onPenPointerHover() {
  const pen = InputManager.tool.data as PenToolData;
  const hasOverlay = pen.overlay ? SceneManager.hasRenderOverlay(pen.overlay.id) : false;

  if (pen.element && pen.vertex) {
    if (
      pen.overlay &&
      pen.overlayVertex &&
      pen.overlayLastVertex &&
      vec2.equals(pen.overlayLastVertex.position, pen.vertex.position)
    ) {
      pen.overlayVertex.translate(
        vec2.sub(
          InputManager.scene.position,
          vec2.add(pen.overlayVertex.position, pen.overlay.position)
        )
      );
    } else {
      if (hasOverlay) SceneManager.popRenderOverlay(pen.overlay!.id);
      pen.overlayLastVertex = new Vertex({
        position: pen.vertex.position,
        left: pen.vertex.left?.position,
        right: pen.vertex.right?.position
      });
      pen.overlayVertex = new Vertex({
        position: vec2.sub(InputManager.scene.position, pen.element.position)
      });
      pen.overlay = new Element({
        position: pen.element.position,
        vertices: [pen.overlayLastVertex, pen.overlayVertex]
      });
    }
    if (!hasOverlay) SceneManager.pushRenderOverlay(pen.overlay);
    SceneManager.render();
  } else if (hasOverlay) {
    SceneManager.popRenderOverlay(pen.overlay!.id);
    pen.overlay = undefined;
    pen.overlayVertex = undefined;
  }
}

export default onPenPointerDown;
