import { vec2 } from '@/math';
import { isBezier } from '../ecs/entities/bezier';
import Element, { isElement } from '../ecs/entities/element';
import { isHandle, isVertexHandle } from '../ecs/entities/handle';
import Pen from '../ecs/entities/pen';
import Vertex from '../ecs/entities/vertex';
import { EntityValue } from '../history/value';
import InputManager from '../input';
import SceneManager from '../scene';
import SelectionManager from '../selection';

function setLeftHandle(vertex: VertexEntity, position: vec2 = [0, 0]) {
  const element = vertex.parent;
  const center = element.transform.center;

  vertex.transform.leftValue = position;
  vertex.pauseCache();

  element.transform.keepCentered(center);
}

function setRightHandle(vertex: VertexEntity, position: vec2 = [0, 0]) {
  const element = vertex.parent;
  const center = element.transform.center;

  vertex.transform.rightValue = position;
  vertex.pauseCache();

  element.transform.keepCentered(center);
}

const onPenNewPointerDown = (pen: PenDataStateInterface) => {
  let pElement = pen.element.value;
  let pVertex = pen.vertex.value;

  const center = pElement?.transform.center;
  const position = vec2.create();

  if (!pElement) {
    pen.element.value = pElement = new Element({
      position: InputManager.scene.position,
      stroke: { color: [0, 0, 0, 1] },
      fill: { color: [1, 1, 1, 1] }
    });

    SceneManager.add(pElement);
  } else {
    const center = pElement.transform.center;
    const angle = pElement.transform.rotation.value;

    vec2.sub(
      angle === 0
        ? InputManager.scene.position
        : vec2.rotate(InputManager.scene.position, center, -angle),
      pElement.transform.position.value,
      position
    );
  }

  pen.vertex.value = pVertex = new Vertex({ position });
  pElement.add(pVertex);

  SelectionManager.clear();
  SelectionManager.select(pElement);

  if (center) pElement.transform.keepCentered(center);

  function onPointerMove(delta: vec2) {
    if (!pVertex) return;

    if (InputManager.keys.space) {
      // TODO: Move vertex
      return;
    }

    if (!InputManager.keys.alt) setLeftHandle(pVertex, vec2.neg(delta));
    setRightHandle(pVertex, delta);
  }

  return {
    onPointerMove
  };
};

const onPenJoinPointerDown = (
  pen: PenDataStateInterface,
  element: ElementEntity,
  vertex: VertexEntity
) => {
  const pElement = pen.element.value;
  if (!pElement) return {};

  if (!element.isFirstVertex(vertex.id)) element.reverse();
  pElement.concat(element);

  pen.vertex.value = vertex;
  SelectionManager.clear();
  SelectionManager.select(pElement);

  function onPointerMove(delta: vec2) {
    setLeftHandle(vertex, vec2.neg(delta));

    if (!InputManager.keys.alt && vertex.transform.right) {
      const direction = vec2.normalize(delta);

      if (!vec2.equals(direction, [0, 0])) {
        setRightHandle(
          vertex,
          vec2.mulS(direction, vec2.len(vertex.transform.right.value || [0, 0]))
        );
      }
    }
  }

  function onPointerUp() {
    pen.element.value = undefined;
    pen.vertex.value = undefined;
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

const onPenClosePointerDown = (
  pen: PenDataStateInterface,
  element: ElementEntity,
  vertex: VertexEntity
) => {
  element.close();

  pen.element.value = element;
  pen.vertex.value = vertex;

  function onPointerMove(delta: vec2) {
    setLeftHandle(vertex, vec2.neg(delta));

    if (!InputManager.keys.alt && vertex.transform.right) {
      const direction = vec2.normalize(delta);

      if (!vec2.equals(direction, [0, 0])) {
        setRightHandle(
          vertex,
          vec2.mulS(direction, vec2.len(vertex.transform.right.value || [0, 0]))
        );
      }
    }
  }

  function onPointerUp() {
    pen.element.value = undefined;
    pen.vertex.value = undefined;
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

const onPenSubPointerDown = (
  pen: PenDataStateInterface,
  element: ElementEntity,
  vertex: VertexEntity
) => {
  function onPointerUp() {
    if (vec2.sqrLen(InputManager.client.delta) < Math.pow(10 / SceneManager.viewport.zoom, 2)) {
      const center = element.transform.center;

      element.remove(vertex, true);
      element.transform.keepCentered(center);
    }

    pen.element.value = undefined;
    pen.vertex.value = undefined;
  }

  return {
    onPointerUp
  };
};

const onPenAddPointerDown = (
  pen: PenDataStateInterface,
  element: ElementEntity,
  bezier: BezierEntity
) => {
  const center = element.transform.center;
  const vertex = element.split(bezier, InputManager.scene.position);
  if (!vertex) return {};

  element.transform.keepCentered(center);

  if (vertex) {
    pen.element.value = element;
    pen.vertex.value = vertex;

    SelectionManager.clear();
    SelectionManager.select(element);
  }

  function onPointerMove(delta: vec2) {
    if (!vertex) return;

    setRightHandle(vertex, delta);
    if (!InputManager.keys.alt) setLeftHandle(vertex, vec2.neg(delta));
  }

  function onPointerUp() {
    pen.element.value = undefined;
    pen.vertex.value = undefined;
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

const onPenAnglePointerDown = (
  pen: PenDataStateInterface,
  element: ElementEntity,
  vertex: VertexEntity
) => {
  pen.element.value = element;
  pen.vertex.value = vertex;
  vertex.right = undefined;

  function onPointerMove(delta: vec2) {
    setRightHandle(vertex, delta);

    if (!InputManager.keys.alt && vertex.transform.left) {
      const direction = vec2.normalize(vec2.neg(delta));

      if (!vec2.equals(direction, [0, 0])) {
        setLeftHandle(vertex, vec2.mulS(direction, vec2.len(vertex.transform.left.value)));
      }
    }
  }

  return {
    onPointerMove
  };
};

const onPenStartPointerDown = (
  pen: PenDataStateInterface,
  element: ElementEntity,
  vertex: VertexEntity
) => {
  if (element.isFirstVertex(vertex.id)) element.reverse();

  pen.element.value = element;
  pen.vertex.value = vertex;
  vertex.right = undefined;

  SelectionManager.clear();
  SelectionManager.select(element);

  function onPointerMove(delta: vec2) {
    setRightHandle(vertex, delta);

    if (!InputManager.keys.alt && vertex.transform.left) {
      const direction = vec2.normalize(vec2.neg(delta));

      if (!vec2.equals(direction, [0, 0])) {
        setLeftHandle(vertex, vec2.mulS(direction, vec2.len(vertex.transform.left.value)));
      }
    }
  }

  return {
    onPointerMove
  };
};

class PenDataState implements PenDataStateInterface {
  readonly element: EntityValue<ElementEntity> = new EntityValue();
  readonly vertex: EntityValue<VertexEntity> = new EntityValue();
  readonly overlay: PenEntity = new Pen();
}

const onPenPointerDown = () => {
  if (!(<PenToolData>InputManager.tool.data).pen)
    (<PenToolData>InputManager.tool.data).pen = new PenDataState();

  const pen = (InputManager.tool.data as PenToolData).pen!;
  const hovered = InputManager.hover.entity;
  const entity = hovered ? InputManager.hover.element : null;
  const element = entity && isElement(entity) ? entity : null;
  const bezier = hovered && isBezier(hovered) ? hovered : null;
  const handle = hovered && isHandle(hovered) ? hovered : null;
  const vertex = handle && isVertexHandle(handle) ? handle.parent : null;
  const overlay = pen.overlay;

  if (overlay) SceneManager.overlays.remove(overlay.id);

  let functions: Partial<ReturnType<typeof onPenClosePointerDown>>;

  if (vertex && element) {
    if (element.isOpenEnd(vertex.id)) {
      if (pen.element.value && element.id === pen.element.value.id) {
        if (pen.vertex.value && vertex.id === pen.vertex.value.id)
          functions = onPenAnglePointerDown(pen, element, vertex);
        else functions = onPenClosePointerDown(pen, element, vertex);
      } else {
        if (pen.element.value) functions = onPenJoinPointerDown(pen, element, vertex);
        else functions = onPenStartPointerDown(pen, element, vertex);
      }
    } else if (SelectionManager.has(element.id))
      functions = onPenSubPointerDown(pen, element, vertex);
  } else if (bezier && element && SelectionManager.has(element.id))
    functions = onPenAddPointerDown(pen, element, bezier);
  else functions = onPenNewPointerDown(pen);

  function onPointerMove() {
    const pElement = pen.element.value;
    if (!pElement) return;

    const unrotatedDelta = InputManager.keys.shift
      ? vec2.snap(InputManager.scene.delta)
      : InputManager.scene.delta;

    const delta =
      pElement.transform.rotation.value !== 0
        ? vec2.rotate(unrotatedDelta, [0, 0], -pElement.transform.rotation.value)
        : unrotatedDelta;

    if (functions.onPointerMove) functions.onPointerMove(delta);
  }

  function onPointerUp() {
    if (functions.onPointerUp) functions.onPointerUp();
    onPenPointerHover();
  }

  return { onPointerMove, onPointerUp };
};

export function onPenPointerHover() {
  if (!(<PenToolData>InputManager.tool.data).pen)
    (<PenToolData>InputManager.tool.data).pen = new PenDataState();

  const pen = (<PenToolData>InputManager.tool.data).pen!;
  const element = pen.element.value;
  const vertex = pen.vertex.value;
  const overlay = pen.overlay;
  const hasOverlay = SceneManager.overlays.has(overlay.id);

  if (element && vertex) {
    const p0 = element.transform.transform(vertex.transform.position.value);
    const p1 = vertex.transform.right
      ? element.transform.transform(vertex.transform.transform(vertex.transform.right.value))
      : undefined;
    const p3 = InputManager.scene.position;

    overlay.set({ p0, p1, p2: undefined, p3 });

    if (!hasOverlay) SceneManager.overlays.add({ entity: overlay });
    SceneManager.render();
  } else if (hasOverlay) SceneManager.overlays.remove(overlay.id);
}

export default onPenPointerDown;
