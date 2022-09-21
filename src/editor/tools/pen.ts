import { vec2 } from '@math';
import { nanoid } from 'nanoid';
import Bezier from '../ecs/entities/bezier';
import Element from '../ecs/entities/element';
import Handle from '../ecs/entities/handle';
import Vertex from '../ecs/entities/vertex';
import HistoryManager from '../history';
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

  HistoryManager.beginSequence();

  if (pen.overlay) SceneManager.popRenderOverlay(pen.overlay!.id);

  pen.overlay = undefined;
  pen.overlayLastVertex = undefined;
  pen.overlayVertex = undefined;

  switch (penState) {
    case 'join': {
      if (!element!.isFirstVertex(vertex!.id)) element!.reverse();

      pen.element!.concat(element!);

      HistoryManager.record({
        fn: () => {
          pen.vertex = vertex;

          SelectionManager.clear();
          SelectionManager.select(pen.element!);
        },
        undo: () => {
          restorePen();
          SelectionManager.restore(backupSelection);
        }
      });

      break;
    }
    case 'close': {
      element!.close();

      HistoryManager.record({
        fn: () => {
          pen.element = element;
          pen.vertex = vertex;

          SelectionManager.clear();
          SelectionManager.select(pen.element!);
        },
        undo: () => {
          restorePen();
          SelectionManager.restore(backupSelection);
        }
      });

      break;
    }
    case 'sub': {
      break;
    }
    case 'add': {
      const vertex = element!.split(bezier!, InputManager.scene.position);

      HistoryManager.record({
        fn: () => {
          if (vertex) {
            pen.element = element;
            pen.vertex = vertex;

            SelectionManager.clear();
            SelectionManager.select(element!);
          }
        },
        undo: () => {
          restorePen();
          SelectionManager.restore(backupSelection);
        }
      });

      break;
    }
    case 'angle': {
      const h = vertex!.right;

      HistoryManager.record({
        fn: () => {
          pen.element = element;
          pen.vertex = vertex;
          vertex!.right = undefined;
        },
        undo: () => {
          vertex!.right = h;
          restorePen();
        }
      });

      break;
    }
    case 'start': {
      if (element!.isFirstVertex(vertex!.id)) element!.reverse();

      const h = vertex!.right;

      HistoryManager.record({
        fn: () => {
          pen.element = element;
          pen.vertex = vertex;
          vertex!.right = undefined;

          SelectionManager.clear();
          SelectionManager.select(element!);
        },
        undo: () => {
          vertex!.right = h;
          restorePen();
          SelectionManager.restore(backupSelection);
        }
      });

      break;
    }
    case 'new': {
      const v = new Vertex({
        position: pen.element
          ? vec2.sub(InputManager.scene.position, pen.element.transform.position)
          : vec2.create()
      });

      if (!pen.element) {
        pen.element = new Element({
          position: InputManager.scene.position,
          stroke: nanoid(),
          fill: nanoid()
        });

        SceneManager.add(pen.element);
      }

      const e = pen.element;

      e.push(v);

      HistoryManager.record({
        fn: () => {
          pen.element = e;
          pen.vertex = v;
          SelectionManager.clear();
          SelectionManager.select(e);
        },
        undo: () => {
          restorePen();
          SelectionManager.restore(backupSelection);
        }
      });

      break;
    }
  }

  function setLeft(position?: vec2, recordHandleCreation = false) {
    if (!pen.vertex) return;

    if (!pen.vertex.left) {
      pen.vertex.transform.left = position || vec2.create();

      if (recordHandleCreation) {
        const v = pen.vertex;
        const backup = pen.vertex.left;

        HistoryManager.record({
          fn: () => {
            v.left = backup;
          },
          undo: () => {
            v.left = undefined;
          }
        });
      }
    } else if (position) pen.vertex.transform.translationLeft = position;
  }

  function setRight(position?: vec2, recordHandleCreation = false) {
    if (!pen.vertex) return;

    if (!pen.vertex.right) {
      pen.vertex.transform.right = position || vec2.create();

      if (recordHandleCreation) {
        const v = pen.vertex;
        const backup = pen.vertex.right;

        HistoryManager.record({
          fn: () => {
            v.right = backup;
          },
          undo: () => {
            v.right = undefined;
          }
        });
      }
    } else if (position) pen.vertex.transform.translationRight = position;
  }

  const left = !!(pen.vertex && pen.vertex.left);
  const right = !!(pen.vertex && pen.vertex.right);

  function onPointerMove() {
    let delta = InputManager.keys.shift
      ? vec2.snap(InputManager.scene.delta)
      : InputManager.scene.delta;

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
          const direction = vec2.unit(delta);

          if (!vec2.equals(direction, [0, 0])) {
            setRight(vec2.mul(direction, vec2.len(pen.vertex!.transform.right)), true);
          }
        }

        break;
      }
      case 'start':
      case 'angle': {
        setRight(delta, true);

        if (!InputManager.keys.alt && left) {
          const direction = vec2.unit(vec2.neg(delta));

          if (!vec2.equals(direction, [0, 0])) {
            setLeft(vec2.mul(direction, vec2.len(pen.vertex!.transform.left)), true);
          }
        }

        break;
      }
      case 'new': {
        if (!InputManager.keys.alt) setLeft(vec2.neg(delta));

        setRight(delta);

        break;
      }
    }
  }

  function onPointerUp() {
    if (pen.vertex) {
      pen.vertex.transform.apply();
    }

    switch (penState) {
      case 'close':
      case 'join':
      case 'add': {
        HistoryManager.record({
          fn: () => {
            pen.element = undefined;
            pen.vertex = undefined;

            onPenPointerHover();
          },
          undo: () => {}
        });

        break;
      }
      case 'sub': {
        if (vec2.len(InputManager.client.delta) < 10 / SceneManager.viewport.zoom)
          element!.delete(vertex!, true);

        HistoryManager.record({
          fn: () => {
            pen.element = undefined;
            pen.vertex = undefined;
          },
          undo: () => {
            restorePen();
            SelectionManager.restore(backupSelection);
          }
        });
        break;
      }
      case 'angle':
      case 'start':
      case 'new': {
        HistoryManager.record({
          fn: () => {
            onPenPointerHover();
          },
          undo: () => {}
        });

        break;
      }
    }

    HistoryManager.endSequence();
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
      vec2.equals(pen.overlayLastVertex.transform.position, pen.vertex.transform.position)
    ) {
      if (pen.vertex.left)
        pen.overlayLastVertex.transform.translationLeft = pen.vertex.transform.left;
      if (pen.vertex.right)
        pen.overlayLastVertex.transform.translationRight = pen.vertex.transform.right;

      pen.overlayVertex.transform.translate(
        vec2.sub(
          InputManager.scene.position,
          vec2.add(pen.overlayVertex.transform.position, pen.overlay.transform.position)
        )
      );
    } else {
      if (hasOverlay) SceneManager.popRenderOverlay(pen.overlay!.id);

      pen.overlayLastVertex = new Vertex({
        position: pen.vertex.transform.position,
        left: pen.vertex.left?.transform.position,
        right: pen.vertex.right?.transform.position
      });

      pen.overlayVertex = new Vertex({
        position: vec2.sub(InputManager.scene.position, pen.element.transform.position)
      });

      pen.overlay = new Element({
        position: pen.element.transform.position,
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
