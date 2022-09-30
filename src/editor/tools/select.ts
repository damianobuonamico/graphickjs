import { BUTTONS } from '@/utils/keys';
import { vec2 } from '@math';
import Element from '../ecs/entities/element';
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
      closed: true
    });
    SceneManager.pushRenderOverlay(rect.element);
  }

  SelectionManager.calculateRenderOverlay();

  function onPointerMove() {
    if ((element && SelectionManager.has(element.id)) || InputManager.keys.alt) {
      if (SelectionManager.size) {
        draggingOccurred = true;
        SelectionManager.forEach((entity) => {
          (entity as Element).transform.tempTranslate(InputManager.scene.movement);
        });
      }
    } else if (rect.element) {
      rect.element.vertices = createVertices('rectangle', InputManager.scene.delta);
      SelectionManager.temp(SceneManager.getEntitiesIn(rect.element.boundingBox));
    }

    SelectionManager.calculateRenderOverlay();
  }

  function onPointerUp(abort?: boolean) {
    SelectionManager.sync();
    if (rect.element) {
      SceneManager.popRenderOverlay(rect.element.id);
      rect.element = undefined;
    }

    if (abort) {
      SelectionManager.forEach((entity) => {
        (entity as Element).transform.clear();
      });
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
