import { vec2 } from '@/math';
import { BUTTONS } from '@/utils/keys';
import { isBezier } from '../ecs/entities/bezier';
import { isElement } from '../ecs/entities/element';
import { isHandle, isVertexHandle } from '../ecs/entities/handle';
import Selector from '../ecs/entities/selector';
import InputManager from '../input';
import SceneManager from '../scene';
import SelectionManager from '../selection';

function translateSelected(movement: vec2) {
  SelectionManager.forEach((entity) => {
    if (!isElement(entity) || entity.selection.full) {
      entity.transform.translate(InputManager.scene.movement);
      return;
    }

    const angle = entity.transform.rotation.value;

    if (angle === 0) entity.selection.forEach((vertex) => vertex.transform.translate(movement));
    else {
      const center = entity.transform.center;
      const mov = vec2.rotate(movement, [0, 0], -angle);

      entity.selection.forEach((vertex) => vertex.transform.translate(mov));
      entity.transform.keepCentered(center);
    }
  });
}

const onDirectSelectElementPointerDown = (
  state: DirectSelectToolStateInterface,
  element: ElementEntity
) => {
  if (!SelectionManager.has(element.id) || !element.selection.full) {
    if (!InputManager.keys.shift) SelectionManager.clear();

    SelectionManager.select(element);
    state.entityIsAddedToSelection = true;
  }

  function onPointerMove(movement: vec2) {
    translateSelected(movement);
  }

  function onPointerUp() {
    if (state.draggingOccurred) SelectionManager.forEach((entity) => entity.transform.apply());
    else if (SelectionManager.has(element.id) && !state.entityIsAddedToSelection) {
      if (InputManager.keys.shift) SelectionManager.deselect(element.id);
      else {
        if (InputManager.button === BUTTONS.LEFT) SelectionManager.clear();
        SelectionManager.select(element, true);
      }
    }
  }

  return { onPointerMove, onPointerUp };
};

const onDirectSelectEntityPointerDown = (state: DirectSelectToolStateInterface, entity: Entity) => {
  if (!SelectionManager.has(entity.id)) {
    if (!InputManager.keys.shift) SelectionManager.clear();

    SelectionManager.select(entity);
    state.entityIsAddedToSelection = true;
  }

  function onPointerMove(movement: vec2) {
    translateSelected(movement);
  }

  function onPointerUp() {
    if (state.draggingOccurred) SelectionManager.forEach((entity) => entity.transform.apply());
    else if (SelectionManager.has(entity.id) && !state.entityIsAddedToSelection) {
      if (InputManager.keys.shift) SelectionManager.deselect(entity.id);
      else {
        if (InputManager.button === BUTTONS.LEFT) SelectionManager.clear();
        SelectionManager.select(entity);
      }
    }
  }

  return { onPointerMove, onPointerUp };
};

const onDirectSelectVertexPointerDown = (
  state: DirectSelectToolStateInterface,
  element: ElementEntity,
  vertex: VertexEntity
) => {
  if (!element.selection.has(vertex.id)) {
    if (!InputManager.keys.shift) SelectionManager.clear();

    element.selection.select(vertex);
    state.entityIsAddedToSelection = true;
  }

  function onPointerMove(movement: vec2) {
    translateSelected(movement);
  }

  function onPointerUp() {
    if (state.draggingOccurred && element.selection.size) {
      SelectionManager.forEach((entity) => entity.transform.apply());
    } else if (element.selection.has(vertex.id) && !state.entityIsAddedToSelection) {
      if (InputManager.keys.shift) element.selection.deselect(vertex.id);
      else {
        if (InputManager.button === BUTTONS.LEFT) SelectionManager.clear();
        element.selection.select(vertex);
      }
    }
  }

  return { onPointerMove, onPointerUp };
};

const onDirectSelectHandlePointerDown = (
  state: DirectSelectToolStateInterface,
  handle: HandleEntity
) => {
  const vertex = handle.parent;
  const element = vertex.parent;

  function onPointerMove(movement: vec2) {
    const angle = element.transform.rotation.value;
    const center = element.transform.center;

    if (InputManager.keys.space) {
      if (angle === 0) vertex.transform.translate(movement);
      else {
        const mov = vec2.rotate(movement, [0, 0], -angle);

        vertex.transform.translate(mov);
        element.transform.keepCentered(center);
      }

      return;
    }

    vec2.rotate(movement, [0, 0], -angle, movement);

    if (handle.id === vertex.left?.id)
      vertex.transform.translateLeft(movement, InputManager.keys.alt);
    else vertex.transform.translateRight(movement, InputManager.keys.alt);

    if (angle !== 0) element.transform.keepCentered(center);
  }

  function onPointerUp() {
    if (state.draggingOccurred) element.transform.apply();
  }

  return { onPointerMove, onPointerUp };
};

const onDirectSelectBezierPointerDown = (
  state: DirectSelectToolStateInterface,
  element: ElementEntity,
  bezier: BezierEntity
) => {
  let shouldEvaluateSelection = false;

  if (
    !SelectionManager.has(element.id) ||
    !element.selection.has(bezier.start.id) ||
    !element.selection.has(bezier.end.id)
  ) {
    if (bezier.bezierType === 'linear') {
      if (!InputManager.keys.shift) SelectionManager.clear();

      element.selection.select(bezier.start);
      element.selection.select(bezier.end);
      state.entityIsAddedToSelection = true;
    } else {
      if (InputManager.keys.shift) {
        element.selection.select(bezier.start);
        element.selection.select(bezier.end);
        state.entityIsAddedToSelection = true;
      } else {
        SelectionManager.clear();
        SelectionManager.select(element, false);
        shouldEvaluateSelection = true;
      }
    }
  }

  let onPointerMove = translateSelected;

  if (shouldEvaluateSelection) {
    const last = element.transform.untransform(InputManager.scene.origin);

    onPointerMove = () => {
      const center = element.transform.center;

      const B = bezier.getClosestTo(last);
      const t = bezier.getClosestTTo(last);
      const { e1, e2 } = bezier.getStrutPoints(t);
      const S = bezier.p0;
      const E = bezier.p3;
      const nB = element.transform.untransform(InputManager.scene.position);
      const d1 = vec2.sub(e1, B);
      const d2 = vec2.sub(e2, B);
      const ne1 = vec2.add(nB, d1);
      const ne2 = vec2.add(nB, d2);
      const { A } = bezier.getABC(t, nB);

      const { p1, p2 } = bezier.deriveControlPoints(A, ne1, ne2, t);

      if (
        bezier.start.transform.left &&
        bezier.start.transform.right &&
        vec2.equals(
          vec2.normalize(bezier.start.transform.left.value),
          vec2.normalize(vec2.neg(bezier.start.transform.right.value))
        )
      ) {
        bezier.start.transform.translateRight(
          vec2.sub(vec2.sub(p1, S), bezier.start.transform.right.value)
        );
      } else bezier.start.transform.rightValue = vec2.sub(p1, S);

      if (
        bezier.end.transform.right &&
        bezier.end.transform.left &&
        vec2.equals(
          vec2.normalize(bezier.end.transform.right.value),
          vec2.normalize(vec2.neg(bezier.end.transform.left.value))
        )
      ) {
        bezier.end.transform.translateLeft(
          vec2.sub(vec2.sub(p2, E), bezier.end.transform.left.value)
        );
      } else bezier.end.transform.leftValue = vec2.sub(p2, E);

      vec2.copy(last, nB);

      if (element.transform.rotation.value !== 0) element.transform.keepCentered(center);
    };
  }

  function onPointerUp() {
    if (state.draggingOccurred) SelectionManager.forEach((entity) => entity.transform.apply());
    else if (shouldEvaluateSelection) {
      element.selection.select(bezier.start);
      element.selection.select(bezier.end);
    } else if (
      !state.entityIsAddedToSelection &&
      SelectionManager.has(element.id) &&
      element.selection.has(bezier.start.id) &&
      element.selection.has(bezier.end.id)
    ) {
      if (InputManager.keys.shift) {
        element.selection.deselect(bezier.start.id);
        element.selection.deselect(bezier.end.id);
      } else {
        if (InputManager.button === BUTTONS.LEFT) SelectionManager.clear();
        element.selection.select(bezier.start);
        element.selection.select(bezier.end);
      }
    }
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

const onDirectSelectNullPointerDown = (
  state: DirectSelectToolStateInterface,
  rect: SelectToolData
) => {
  if (!InputManager.keys.shift) SelectionManager.clear();

  const selector = (rect.selector = new Selector({
    position: InputManager.scene.position,
    dashed: true
  }));

  SceneManager.overlays.add({ entity: rect.selector });

  function onPointerMove() {
    state.draggingOccurred = false;

    selector.set(InputManager.scene.delta);

    const box = selector.transform.boundingBox;
    const entities = SceneManager.getEntitiesIn(box);

    entities.forEach((entity) => {
      if (isElement(entity)) {
        const vertices = new Set<VertexEntity>();
        entity.getEntitiesIn(box, vertices, true);

        entity.selection.temp(vertices);
        if (!entity.selection.size) entity.selection.tempAll();
      }
    });

    SelectionManager.temp(entities);
  }

  function onPointerUp() {}

  return {
    onPointerMove,
    onPointerUp
  };
};

const onDirectSelectPointerDown = () => {
  const rect = <SelectToolData>InputManager.tool.data;
  const hovered = InputManager.hover.entity;
  const entity = hovered ? InputManager.hover.element : null;
  const element = entity && isElement(entity) ? entity : null;
  const bezier = hovered && isBezier(hovered) ? hovered : null;
  const handle = hovered && isHandle(hovered) ? hovered : null;
  const vertex = handle && isVertexHandle(handle) ? handle.parent : null;

  let state: DirectSelectToolStateInterface = {
    draggingOccurred: false,
    entityIsAddedToSelection: false
  };

  let functions: Partial<ReturnType<typeof onDirectSelectHandlePointerDown>> = {};

  if (InputManager.keys.alt && entity && (!handle || vertex)) {
    if (!SelectionManager.has(entity.id)) {
      if (!InputManager.keys.shift) SelectionManager.clear();

      SelectionManager.select(entity);
      state.entityIsAddedToSelection = true;
    }

    let entities: Entity[] | null = null;

    functions.onPointerMove = (movement: vec2) => {
      if (!entities) {
        entities = SelectionManager.entities;
        SelectionManager.clear();

        entities.forEach((entity) => {
          const duplicate = SceneManager.duplicate(entity);
          if (duplicate) SelectionManager.select(duplicate);
        });
      }

      translateSelected(movement);
    };

    functions.onPointerUp = () => {
      if (state.draggingOccurred) SelectionManager.forEach((entity) => entity.transform.apply());
    };
  } else if (element) {
    if (vertex) functions = onDirectSelectVertexPointerDown(state, element, vertex);
    else if (handle) functions = onDirectSelectHandlePointerDown(state, handle);
    else if (bezier) functions = onDirectSelectBezierPointerDown(state, element, bezier);
    else functions = onDirectSelectElementPointerDown(state, element);
  } else if (entity && entity.selectable)
    functions = onDirectSelectEntityPointerDown(state, entity);
  else functions = onDirectSelectNullPointerDown(state, rect);

  function onPointerMove() {
    state.draggingOccurred = true;

    if (functions.onPointerMove) functions.onPointerMove(InputManager.scene.movement);
  }

  function onPointerUp() {
    if (functions.onPointerUp) functions.onPointerUp();
  }

  return { onPointerMove, onPointerUp };
};

export default onDirectSelectPointerDown;
