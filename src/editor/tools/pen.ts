import { vec2 } from '@math';
import Bezier from '../ecs/entities/bezier';
import Element from '../ecs/entities/element';
import Handle from '../ecs/entities/handle';
import Vertex from '../ecs/entities/vertex';
import CommandHistory from '../history/history';
import InputManager from '../input';
import SceneManager from '../scene';
import SelectionManager from '../selection';

interface PenToolData {
  vertex?: VertexEntity;
  element?: Element;
  overlay?: Element;
  overlayLastVertex?: Vertex;
  overlayVertex?: Vertex;
}

const onPenPointerDown = () => {
  const pen = InputManager.tool.data as PenToolData;
  const entity = InputManager.hover.entity;
  const el = InputManager.hover.element;
  const element = el && el.type === 'element' ? (el as Element) : undefined;
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

  const backupPen: PenToolData = { ...pen };
  const backupSelection: SelectionBackup = SelectionManager.get();

  function restorePen() {
    pen.vertex = backupPen.vertex;
    pen.element = backupPen.element;
    pen.overlay = backupPen.overlay;
    pen.overlayVertex = backupPen.overlayVertex;
    pen.overlayLastVertex = backupPen.overlayLastVertex;
  }

  if (pen.overlay) SceneManager.overlays.remove(pen.overlay!.id);

  pen.overlay = undefined;
  pen.overlayLastVertex = undefined;
  pen.overlayVertex = undefined;

  switch (penState) {
    case 'join': {
      if (!element!.isFirstVertex(vertex!.id)) element!.reverse();

      pen.element!.concat(element!);

      // TOCHECK
      pen.vertex = vertex;
      SelectionManager.clear();
      SelectionManager.select(pen.element!);

      // restorePen();
      // SelectionManager.restore(backupSelection);

      break;
    }
    case 'close': {
      element!.close();

      // TOCHECK
      pen.element = element;
      pen.vertex = vertex;

      SelectionManager.clear();
      SelectionManager.select(pen.element!);

      // restorePen();
      // SelectionManager.restore(backupSelection);

      break;
    }
    case 'sub': {
      break;
    }
    case 'add': {
      const center = element!.transform.center;
      const vertex = element!.split(bezier!, InputManager.scene.position);
      element!.transform.keepCentered(center);

      // TOCHECK
      if (vertex) {
        pen.element = element;
        pen.vertex = vertex;

        SelectionManager.clear();
        SelectionManager.select(element!);
      }

      // restorePen();
      // SelectionManager.restore(backupSelection);

      break;
    }
    case 'angle': {
      // TOCHECK
      pen.element = element;
      pen.vertex = vertex;
      vertex!.right = undefined;

      // restorePen();

      break;
    }
    case 'start': {
      if (element!.isFirstVertex(vertex!.id)) element!.reverse();

      // TOCHECK
      pen.element = element;
      pen.vertex = vertex;
      vertex!.right = undefined;

      SelectionManager.clear();
      SelectionManager.select(element!);

      // restorePen();
      // SelectionManager.restore(backupSelection);

      break;
    }
    case 'new': {
      const center = pen.element?.transform.center;
      const angle = pen.element?.transform.rotation.value;

      const delta =
        pen.element &&
        vec2.sub(
          angle === 0
            ? InputManager.scene.position
            : vec2.rotate(InputManager.scene.position, center!, -(angle || 0)),
          pen.element.transform.position.value
        );

      // if (InputManager.keys.shift && delta) vec2.snap(delta, 8, delta);

      const v = new Vertex({
        position: delta || vec2.create()
      });

      if (!pen.element) {
        pen.element = new Element({
          position: InputManager.scene.position,
          stroke: { color: [0, 0, 0, 1] },
          fill: {}
        });

        SceneManager.add(pen.element);
      }

      const e = pen.element;

      e.add(v);

      if (center) e.transform.keepCentered(center);

      // TOCHECK
      pen.element = e;
      pen.vertex = v;
      SelectionManager.clear();
      SelectionManager.select(e);

      // restorePen();
      // SelectionManager.restore(backupSelection);

      break;
    }
  }

  function setLeft(position?: vec2, recordHandleCreation = false) {
    if (!pen.vertex) return;

    const center = pen.element!.transform.center;

    if (!pen.vertex.transform.left) {
      pen.vertex.transform.leftValue = position || vec2.create();

      if (recordHandleCreation) {
        const v = pen.vertex;
        const backup = pen.vertex.left;

        v.left = backup;
      }
    } else if (position) pen.vertex.transform.left.value = position;

    pen.vertex.pauseCache();
    pen.element!.transform.keepCentered(center);
  }

  function setRight(position?: vec2, recordHandleCreation = false) {
    if (!pen.vertex) return;

    const center = pen.element!.transform.center;

    if (!pen.vertex.transform.right) {
      pen.vertex.transform.rightValue = position || vec2.create();

      if (recordHandleCreation) {
        const v = pen.vertex;
        const backup = pen.vertex.right;

        // TOCHECK
        v.right = backup;

        // v.right = undefined;
      }
    } else if (position) pen.vertex.transform.right.value = position;

    pen.vertex.pauseCache();
    pen.element!.transform.keepCentered(center);
  }

  const left = !!(pen.vertex && pen.vertex.left);
  const right = !!(pen.vertex && pen.vertex.right);

  function onPointerMove() {
    const unrotatedDelta = InputManager.keys.shift
      ? vec2.snap(InputManager.scene.delta)
      : InputManager.scene.delta;

    const delta =
      pen.element && pen.element.transform.rotation.value !== 0
        ? vec2.rotate(unrotatedDelta, [0, 0], -pen.element.transform.rotation)
        : unrotatedDelta;

    switch (penState) {
      case 'sub':
        break;
      case 'add': {
        setRight(delta);

        if (!InputManager.keys.alt) setLeft(vec2.neg(delta));

        break;
      }
      case 'close':
      case 'join': {
        setLeft(vec2.neg(delta), true);

        if (!InputManager.keys.alt && right) {
          const direction = vec2.normalize(delta);

          if (!vec2.equals(direction, [0, 0])) {
            setRight(
              vec2.mulS(direction, vec2.len(pen.vertex!.transform.right?.value || [0, 0])),
              true
            );
          }
        }

        break;
      }
      case 'start':
      case 'angle': {
        setRight(delta, true);

        if (!InputManager.keys.alt && left) {
          const direction = vec2.normalize(vec2.neg(delta));

          if (!vec2.equals(direction, [0, 0])) {
            setLeft(
              vec2.mulS(direction, vec2.len(pen.vertex!.transform.left?.value || [0, 0])),
              true
            );
          }
        }

        break;
      }
      case 'new': {
        // TODO: if space key is held down, the entire vertex should move
        if (!InputManager.keys.alt) setLeft(vec2.neg(delta));

        setRight(delta);

        break;
      }
    }
  }

  function onPointerUp() {
    if (pen.vertex) pen.vertex.transform.apply();
    if (pen.element) pen.element?.transform.apply();

    switch (penState) {
      case 'close':
      case 'join':
      case 'add': {
        pen.element = undefined;
        pen.vertex = undefined;

        onPenPointerHover();

        break;
      }
      case 'sub': {
        if (vec2.len(InputManager.client.delta) < 10 / SceneManager.viewport.zoom) {
          const center = element!.transform.center;
          element!.remove(vertex!, true);

          element!.transform.keepCentered(center);
        }

        // TOCHECK
        pen.element = undefined;
        pen.vertex = undefined;

        // restorePen();
        // SelectionManager.restore(backupSelection);

        break;
      }
      case 'angle':
      case 'start':
      case 'new': {
        onPenPointerHover();

        break;
      }
    }
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

export function onPenPointerHover() {
  const pen = InputManager.tool.data as PenToolData;
  const hasOverlay = pen.overlay ? SceneManager.overlays.has(pen.overlay.id) : false;

  if (pen.element && pen.vertex) {
    const box = pen.element.transform.unrotatedBoundingBox;
    const mid = vec2.mid(
      vec2.sub(box[0], pen.element.transform.position.value),
      vec2.sub(box[1], pen.element.transform.position.value)
    );

    if (
      pen.overlay &&
      pen.overlayVertex &&
      pen.overlayLastVertex &&
      vec2.equals(
        pen.overlayLastVertex.transform.position.value,
        vec2.rotate(pen.vertex.transform.position.value, mid, pen.element.transform.rotation.value)
      )
    ) {
      if (pen.vertex.transform.left)
        pen.overlayLastVertex.transform.leftValue = vec2.rotate(
          pen.vertex.transform.left.value,
          [0, 0],
          pen.element.transform.rotation.value
        );
      if (pen.vertex.transform.right)
        pen.overlayLastVertex.transform.rightValue = vec2.rotate(
          pen.vertex.transform.right.value,
          [0, 0],
          pen.element.transform.rotation.value
        );

      CommandHistory.ignoreNext();
      pen.overlayVertex.transform.translate(
        vec2.sub(
          InputManager.scene.position,
          vec2.add(pen.overlayVertex.transform.position.value, pen.overlay.transform.position.value)
        )
      );
      CommandHistory.clearIgnore();
    } else {
      if (hasOverlay) SceneManager.overlays.remove(pen.overlay!.id);

      pen.overlayLastVertex = new Vertex({
        position: vec2.rotate(
          pen.vertex.transform.position.value,
          mid,
          pen.element.transform.rotation.value
        ),
        left: pen.vertex.transform.left
          ? vec2.rotate(
              pen.vertex.transform.left.value,
              [0, 0],
              pen.element.transform.rotation.value
            )
          : undefined,
        right: pen.vertex.transform.right
          ? vec2.rotate(
              pen.vertex.transform.right.value,
              [0, 0],
              pen.element.transform.rotation.value
            )
          : undefined
      });

      pen.overlayVertex = new Vertex({
        position: vec2.sub(InputManager.scene.position, pen.element.transform.position.value)
      });

      pen.overlay = new Element({
        position: pen.element.transform.position.value,
        vertices: [pen.overlayLastVertex, pen.overlayVertex],
        stroke: {
          color: [56 / 255, 195 / 255, 242 / 255, 1],
          width: 1.5 / SceneManager.viewport.zoom
        },
        closed: false
      });
    }

    if (!hasOverlay) SceneManager.overlays.add({ entity: pen.overlay });

    SceneManager.render();
  } else if (hasOverlay) {
    SceneManager.overlays.remove(pen.overlay!.id);

    pen.overlay = undefined;
    pen.overlayVertex = undefined;
  }
}

export default onPenPointerDown;
