import { BUTTONS } from '@utils/keys';
import { vec2 } from '@math';
import Bezier from '../ecs/bezier';
import Element from '../ecs/element';
import Handle from '../ecs/handle';
import Vertex from '../ecs/vertex';
import InputManager from '../input';
import { createVertices } from '../renderer/geometry';
import SceneManager from '../scene';
import SelectionManager from '../selection';

interface SelectToolData {
  element?: Element;
}

// TODO: curve selection
const onVSelectPointerDown = () => {
  const entity = InputManager.hover.entity;
  const element = InputManager.hover.element;
  const bezier = entity && entity.type === 'bezier' ? (entity as Bezier) : undefined;
  const handle = entity && entity.type === 'handle' ? (entity as Handle) : undefined;
  const vertex = handle && handle.handleType === 'vertex' ? (handle.parent as Vertex) : undefined;
  const rect = InputManager.tool.data as SelectToolData;

  let draggingOccurred = false;
  let elementIsAddedToSelection = false;

  if (element) {
    if (vertex) {
      if (!element.selection.has(vertex.id)) {
        if (!InputManager.keys.shift) SelectionManager.clear();
        element.selection.select(vertex);
        elementIsAddedToSelection = true;
      }
    } else if (handle) {
    } else {
      if (!element.selection.full) {
        if (!InputManager.keys.shift) {
          SelectionManager.clear();
        }
        elementIsAddedToSelection = true;
      }
      element.selection.all();
      if (InputManager.keys.alt) {
        const entities = SelectionManager.entities;
        SelectionManager.clear();
        entities.forEach((entity) => {
          const duplicate = SceneManager.duplicate(entity);
          if (duplicate) SelectionManager.select(duplicate);
        });
      }
    }
  } else {
    if (!InputManager.keys.shift) {
      SelectionManager.clear();
    }
    rect.element = new Element({
      position: InputManager.scene.position,
      vertices: createVertices('rectangle', vec2.create()),
      closed: true
    });
    SceneManager.pushRenderOverlay(rect.element);
  }

  function onPointerMove() {
    draggingOccurred = true;
    if (element) {
      if (handle && !vertex) {
        if (InputManager.keys.space) {
          handle.parent.translate(InputManager.scene.movement);
        } else {
          handle.translate(InputManager.scene.movement);
        }
      } else {
        SelectionManager.forEach((element) => {
          if ((element as Element).selection.size) {
            (element as Element).selection.forEach((vertex) => {
              vertex.translate(InputManager.scene.movement);
            });
          }
        });
      }
    } else if (rect.element) {
      draggingOccurred = false;
      rect.element.vertices = createVertices('rectangle', InputManager.scene.delta);
      const box = rect.element.boundingBox;
      const entities = SceneManager.getEntitiesIn(box);
      entities.forEach((entity) => {
        if (entity.type === 'element') {
          const vertices = new Set<Vertex>();
          (entity as Element).getEntitiesIn(box, vertices, true);
          (entity as Element).selection.temp(vertices);
          if (!(entity as Element).selection.size) entities.delete(entity);
        }
      });
      SelectionManager.temp(entities);
    }
  }

  function onPointerUp(abort?: boolean) {
    SelectionManager.sync(true);
    if (rect.element) {
      SceneManager.popRenderOverlay(rect.element.id);
      rect.element = undefined;
    }

    if (element) {
      if (vertex) {
        if (abort) {
          SelectionManager.forEach((element) => {
            (element as Element).selection.forEach((vertex) => {
              vertex.clearTransform();
            });
          });
        } else if (draggingOccurred && element.selection.size) {
          SelectionManager.forEach((element) => {
            (element as Element).selection.forEach((vertex) => {
              vertex.applyTransform();
            });
          });
        } else if (element.selection.has(vertex.id) && !elementIsAddedToSelection) {
          if (InputManager.keys.shift) {
            element.selection.deselect(vertex.id);
          } else {
            if (InputManager.button === BUTTONS.LEFT) SelectionManager.clear();
            element.selection.select(vertex);
          }
        }
      } else if (handle) {
        if (abort) handle.clearTransform();
        else handle.applyTransform();
      } else {
        if (abort) {
          SelectionManager.forEach((element) => {
            (element as Element).selection.forEach((vertex) => {
              vertex.clearTransform();
            });
          });
        } else if (draggingOccurred && element.selection.size) {
          SelectionManager.forEach((element) => {
            (element as Element).selection.forEach((vertex) => {
              vertex.applyTransform();
            });
          });
        }
      }
    }
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

export default onVSelectPointerDown;
