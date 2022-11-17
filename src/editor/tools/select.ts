import { GEOMETRY_MAX_ERROR } from '@/utils/constants';
import { BUTTONS } from '@/utils/keys';
import { vec2 } from '@math';
import Element from '../ecs/entities/element';
import CommandHistory from '../history/history';
import InputManager from '../input';
import { createVertices } from '../renderer/geometry';
import SceneManager from '../scene';
import SelectionManager from '../selection';

interface SelectToolData {
  element?: Element;
}

const onSelectPointerDown = () => {
  let draggingOccurred = false;
  let elementIsAddedToSelection = false;
  const element = InputManager.hover.element;
  const rect = InputManager.tool.data as SelectToolData;

  if (!InputManager.keys.shift && (!element || !SelectionManager.has(element.id))) {
    SelectionManager.clear();
  }

  if (element) {
    if (!SelectionManager.has(element.id)) {
      SelectionManager.select(element);
      elementIsAddedToSelection = true;
    }
    if (InputManager.keys.alt) {
      const entities = SelectionManager.entities;
      SelectionManager.clear();
      entities.forEach((entity) => {
        const duplicate = SceneManager.duplicate(entity);
        if (duplicate) SelectionManager.select(duplicate);
      });
    }
  } else {
    rect.element = new Element({
      position: InputManager.scene.position,
      vertices: createVertices('rectangle', vec2.create()),
      closed: true,
      stroke: { color: [56 / 255, 195 / 255, 242 / 255, 1], width: 2 / SceneManager.viewport.zoom },
      fill: { color: [56 / 255, 195 / 255, 242 / 255, 0.2] },
    });
    SceneManager.overlays.add({ entity: rect.element });
  }

  SelectionManager.calculateRenderOverlay();

  function onPointerMove() {
    if ((element && SelectionManager.has(element.id)) || InputManager.keys.alt) {
      if (SelectionManager.size) {
        let delta = InputManager.scene.delta;

        const box = SelectionManager.staticBoundingBox;
        vec2.add(box[0], delta, box[0]);
        vec2.add(box[1], delta, box[1]);
        const mid = vec2.mid(box[0], box[1]);
        box.push(mid);

        const THRESH = 4 / SceneManager.viewport.zoom;

        let snappingDelta = [THRESH * 2, THRESH * 2];

        // TODO: preserve shift snapping direction

        SceneManager.forEach((entity) => {
          if (SceneManager.isVisible(entity) && !SelectionManager.ids.includes(entity.id)) {
            const entityBox = entity.transform.boundingBox;

            if (entityBox) {
              entityBox.push(vec2.mid(entityBox[0], entityBox[1]));

              for (let axis = 0; axis < 2; axis++) {
                for (let side1 = 0; side1 < 3; side1++) {
                  for (let side = 0; side < 3; side++) {
                    let dist = entityBox[side][axis] - box[side1][axis];
                    if (Math.abs(dist) < THRESH && Math.abs(dist) < Math.abs(snappingDelta[axis])) {
                      snappingDelta[axis] = dist;
                    }
                  }
                }
              }
            }
          }
        });

        let snappedDelta = vec2.clone(delta);

        for (let axis = 0; axis < 2; axis++) {
          if (
            Math.abs(snappingDelta[axis]) < THRESH &&
            Math.abs(snappingDelta[axis]) > GEOMETRY_MAX_ERROR
          ) {
            delta[axis] += snappingDelta[axis];
            snappedDelta[axis] = delta[axis];
          }
        }

        // TODO: prefere horizontal/vertical snapping
        if (InputManager.keys.shift) vec2.snap(delta, undefined, delta);

        if (delta[1] === 0) delta[0] = snappedDelta[0];
        if (delta[0] === 0) delta[1] = snappedDelta[1];

        draggingOccurred = true;
        SelectionManager.forEach((entity) => {
          (entity as Element).transform.translate(
            vec2.sub(delta, (entity as Element).transform.position.delta)
          );
        });
      }
    } else if (rect.element) {
      rect.element.vertices = createVertices('rectangle', InputManager.scene.delta);
      SelectionManager.temp(SceneManager.getEntitiesIn(rect.element.transform.boundingBox));
    }

    SelectionManager.calculateRenderOverlay();
  }

  function onPointerUp(abort?: boolean) {
    SelectionManager.sync();
    if (rect.element) {
      SceneManager.overlays.remove(rect.element.id);
      rect.element = undefined;
    }

    if (abort) {
      CommandHistory.undo();
      CommandHistory.seal();
    } else if (draggingOccurred && SelectionManager.size) {
      SelectionManager.forEach((entity) => {
        (entity as Element).transform.apply();
      });
    } else if (element && SelectionManager.has(element.id) && !elementIsAddedToSelection) {
      if (InputManager.keys.shift) {
        SelectionManager.deselect(element.id);
      } else {
        if (InputManager.button === BUTTONS.LEFT) SelectionManager.clear();
        SelectionManager.select(element);
      }
    }

    SelectionManager.calculateRenderOverlay();
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

export default onSelectPointerDown;
