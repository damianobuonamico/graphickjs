import { vec2 } from '@math';
import Bezier from '../ecs/bezier';
import Element from '../ecs/element';
import Handle from '../ecs/handle';
import Vertex from '../ecs/vertex';
import InputManager from '../input';
import SceneManager from '../scene';
import SelectionManager from '../selection';

const onPenPointerDown = () => {
  const entity = InputManager.hover.entity;
  const element = InputManager.hover.element;
  const bezier = entity && entity.type === 'bezier' ? (entity as Bezier) : undefined;
  const handle = entity && entity.type === 'handle' ? (entity as Handle) : undefined;
  const vertex = handle && handle.handleType === 'vertex' ? (handle.parent as Vertex) : undefined;
  const pen = InputManager.toolData as PenToolData;
  let last: Vertex | null = null;
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
  else penState = 'new';

  switch (penState) {
    case 'join': {
      /*
      const order = Array.from(element!.vertices.keys());
      if (order[0] !== vertex!.id) {
        order.reverse();
        for (const id of order) {
          const __v = element!.vertices.get(id)!;
          [__v.lBez, __v.rBez] = [__v.rBez, __v.lBez];
          if (__v.lBez) __v.lBez.type = 'lBez';
          if (__v.rBez) __v.rBez.type = 'rBez';
          __v._pos.pos = sub(add(__v._pos.pos, element!.pos), input.pen!.element.pos);
          input.pen!.element.vertices.set(id, __v);
        }
      } else {
        for (const id of order) {
          const __v = element!.vertices.get(id)!;
          __v._pos.pos = sub(add(__v._pos.pos, element!.pos), input.pen!.element.pos);
          input.pen!.element.vertices.set(id, __v);
        }
      }
      input.pen!.element.regenerateStrip();
      elements.selection.clear();
      elements.delete(element!.id);
      elements.selection.select({ element: input.pen!.element, object: false, vertex: vertex! });
      input.pen!.vertex = vertex!;
      */
      break;
    }
    case 'close': {
      (pen.element as Element).close();
      (pen.element as Element).generateCurves();
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
      /*
      input.pen!.vertex.rBez = null;
      */
      break;
    }
    case 'start': {
      /*
      input.pen = { element: element!, vertex: vertex! };
      const order = Array.from(input.pen!.element.vertices.keys());
      if (order[0] === vertex!.id) {
        for (const id of order) {
          const vertex = input.pen!.element.vertices.get(id)!;
          [vertex.lBez, vertex.rBez] = [vertex.rBez, vertex.lBez];
          if (vertex.lBez) vertex.lBez.type = 'lBez';
          if (vertex.rBez) vertex.rBez.type = 'rBez';
        }
        input.pen!.element.regenerateStrip(order.reverse());
      }

      elements.selection.clear();
      elements.selection.select({ element: input.pen.element, object: false, vertex: vertex! });
      input.pen!.vertex.rBez = null;
      */
      break;
    }
    default: {
      last = new Vertex({
        position: pen.element
          ? vec2.sub(InputManager.scene.position, (pen.element as Element).position)
          : vec2.create()
      });
      pen.vertex = last;
      if (!pen.element) {
        pen.element = new Element({
          position: InputManager.scene.position
        });
        SceneManager.add(pen.element);
      }
      // last._pos.pos = sub(api.input.pos, input.pen.element.pos);
      (pen.element as Element).pushVertex(last);
      SelectionManager.clear();
      SelectionManager.select(pen.element);
    }
  }

  function setRight(position?: vec2) {
    if (!pen.vertex) return;
    if (!(pen.vertex as Vertex).right) (pen.vertex as Vertex).setRight(position || vec2.create());
    else if (position) (pen.vertex as Vertex).right!.position = position;
  }

  function setLeft(position?: vec2) {
    if (!pen.vertex) return;
    if (!(pen.vertex as Vertex).left) (pen.vertex as Vertex).setLeft(position || vec2.create());
    else if (position) (pen.vertex as Vertex).left!.position = position;
  }

  const left = !!(pen.vertex && (pen.vertex as Vertex).left);
  const right = !!(pen.vertex && (pen.vertex as Vertex).right);

  function onPointerMove() {
    switch (penState) {
      case 'sub':
        break;
      case 'add': {
        /*
        input.pen!.vertex.rBez!.pos = sub(api.input.pos, api.input.origin);
        if (!api.input.keys.alt) input.pen!.vertex.lBez!.pos = sub(api.input.origin, api.input.pos);
        input.pen!.element.recalculate();
        */
        break;
      }
      case 'close': {
        /*
        createLeft();
        input.pen!.vertex.lBez!.pos = sub(api.input.origin, api.input.pos);
        if (!api.input.keys.alt && right) {
          const direction = unit(sub(api.input.pos, api.input.origin));
          if (!equal(direction, [0, 0])) {
            input.pen!.vertex.rBez!.pos = mul(direction, len(input.pen!.vertex.rBez!.pos));
          }
        }
        input.pen!.element.recalculate();
        */
        break;
      }
      case 'join': {
        /*
        createLeft();
        createRight();
        input.pen!.vertex.lBez!.pos = sub(api.input.origin, api.input.pos);
        if (input.pen!.vertex.rBez && !api.input.keys.alt) {
          const direction = unit(sub(api.input.pos, api.input.origin));
          if (!equal(direction, [0, 0])) {
            input.pen!.vertex.rBez!.pos = mul(unit(direction), len(input.pen!.vertex.rBez!.pos));
          }
        }
        input.pen!.element.recalculate();
        */
        break;
      }
      case 'start':
      case 'angle': {
        /*
        createLeft();
        createRight();
        input.pen!.vertex.rBez!.pos = sub(api.input.pos, api.input.origin);
        if (input.pen!.vertex.lBez && !api.input.keys.alt) {
          const direction = unit(sub(api.input.origin, api.input.pos));
          if (!equal(direction, [0, 0])) {
            input.pen!.vertex.lBez!.pos = mul(direction, len(input.pen!.vertex.lBez!.pos));
          }
        }
        input.pen!.element.recalculate();
        */
        break;
      }
      default:
        {
          if (!InputManager.keys.alt) setLeft(vec2.neg(InputManager.scene.delta));
          setRight(InputManager.scene.delta);
        }
        (pen.element as Element).recalculate();
    }
  }

  function onPointerUp() {
    /*switch (penState) {
      case 'close':
      case 'join':
      case 'add': {
        input.pen = null;
        input.clearCache();
        break;
      }
      case 'sub': {
        if (dist(api.input.pos, api.input.origin) < 10 / state.zoom)
          element!.removeVertex(vertex!.id);
        input.pen = null;
        break;
      }
      case 'angle':
      case 'start':
      default: {
      }
    }*/
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

export default onPenPointerDown;
