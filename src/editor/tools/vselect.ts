import { BUTTONS } from '@utils/keys';
import { vec2 } from '@math';
import Bezier from '../ecs/entities/bezier';
import Element from '../ecs/entities/element';
import Handle from '../ecs/entities/handle';
import Vertex from '../ecs/entities/vertex';
import InputManager from '../input';
import { createVertices } from '../renderer/geometry';
import SceneManager from '../scene';
import SelectionManager from '../selection';
import CommandHistory from '../history/history';

interface SelectToolData {
  element?: Element;
}

// TODO: curve selection
const onVSelectPointerDown = () => {
  const entity = InputManager.hover.entity;
  const el = InputManager.hover.element;
  const element = el && el.type === 'element' ? (el as Element) : null;
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
      closed: true,
      stroke: {
        color: [56 / 255, 195 / 255, 242 / 255, 1],
        width: 2 / SceneManager.viewport.zoom,
        style: [3 / SceneManager.viewport.zoom, 3 / SceneManager.viewport.zoom]
      },
      fill: { color: [56 / 255, 195 / 255, 242 / 255, 0.2] }
    });

    SceneManager.overlays.add({ entity: rect.element });
  }

  let last = InputManager.scene.position;

  function onPointerMove() {
    draggingOccurred = true;
    if (element) {
      if (handle && !vertex) {
        if (InputManager.keys.space) {
          const angle = handle.parent.parent.transform.rotation.value;

          if (angle === 0) {
            handle.parent.transform.translate(InputManager.scene.movement);
          } else {
            const center = (handle.parent.parent.transform as ElementTransformComponent).center;
            const movement = vec2.rotate(InputManager.scene.movement, [0, 0], -angle);

            handle.parent.transform.translate(movement);
            (handle.parent.parent.transform as ElementTransformComponent).keepCentered(
              center,
              true
            );
          }
        } else {
          const angle = handle.parent.parent.transform.rotation.value;
          const center = (handle.parent.parent.transform as ElementTransformComponent).center;

          let value = vec2.rotate(
            vec2.sub(
              InputManager.scene.position,
              vec2.rotate(
                vec2.add(
                  handle.parent.transform.position.value,
                  handle.parent.parent.transform.position.value
                ),
                center,
                angle
              )
            ),
            [0, 0],
            -angle
          );

          if (InputManager.keys.shift) vec2.snap(value, undefined, value);

          if (angle === 0) {
            if (handle.id === handle.parent.left?.id)
              handle.parent.transform.translateLeft(
                vec2.sub(value, handle.parent.transform.left?.value || [0, 0]),
                InputManager.keys.alt
              );
            else
              handle.parent.transform.translateRight(
                vec2.sub(value, handle.parent.transform.right?.value || [0, 0]),
                InputManager.keys.alt
              );
          } else {
            if (handle.id === handle.parent.left?.id)
              handle.parent.transform.translateLeft(
                vec2.sub(value, handle.parent.transform.left?.value || [0, 0]),
                InputManager.keys.alt
              );
            else
              handle.parent.transform.translateRight(
                vec2.sub(value, handle.parent.transform.right?.value || [0, 0]),
                InputManager.keys.alt
              );

            (handle.parent.parent.transform as ElementTransformComponent).keepCentered(
              center,
              true
            );
          }
        }
      } else {
        let current = InputManager.scene.position;

        if (InputManager.keys.shift)
          current = vec2.add(InputManager.scene.origin, vec2.snap(InputManager.scene.delta));

        const movement = vec2.sub(current, last);

        draggingOccurred = true;

        SelectionManager.forEach((el) => {
          if (!vertex && el === element) {
            (el as Element).transform.translate(InputManager.scene.movement);
            return;
          }

          const angle = (el as Element).transform.rotation.value;

          if (angle === 0) {
            if ((el as Element).selection && (el as Element).selection.size) {
              (el as Element).selection.forEach((vertex) => {
                vertex.transform.translate(movement);
              });
            }
          } else {
            const center = (el as Element).transform.center;
            const mov = vec2.rotate(movement, [0, 0], -angle);

            if ((el as Element).selection.size) {
              (el as Element).selection.forEach((vertex) => {
                vertex.transform.translate(mov);
              });
            }

            (el.transform as ElementTransformComponent).keepCentered(center, true);
          }
        });

        last = current;
      }
    } else if (rect.element) {
      draggingOccurred = false;
      rect.element.vertices = createVertices('rectangle', InputManager.scene.delta);
      const box = rect.element.transform.boundingBox;
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
      SceneManager.overlays.remove(rect.element.id);
      rect.element = undefined;
    }

    if (element) {
      if (vertex) {
        if (abort) {
          CommandHistory.undo();
          CommandHistory.seal();
        } else if (draggingOccurred && element.selection.size) {
          SelectionManager.forEach((element) => {
            (element as Element).selection.forEach((vertex) => {
              vertex.transform.apply();
            });

            (element as Element).transform.apply();
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
        if (abort) {
          CommandHistory.undo();
          CommandHistory.seal();
        } else {
          handle.parent.transform.apply();
          handle.parent.parent.transform.apply();
        }
      } else {
        if (abort) {
          CommandHistory.undo();
          CommandHistory.seal();
        } else if (draggingOccurred && element.selection.size) {
          SelectionManager.forEach((element) => {
            (element as Element).selection.forEach((vertex) => {
              vertex.transform.apply();
            });
          });

          (element as Element).transform.apply();
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
