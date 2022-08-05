import { BUTTONS } from '@/utils/keys';
import { vec2 } from '@math';
import InputManager from '../input';
import SceneManager from '../scene';
import SelectionManager from '../selection';

const onSelectPointerDown = () => {
  let draggingOccurred = false;
  let elementIsAddedToSelection = false;
  const element = InputManager.hover.element;

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
        console.log(duplicate);
        if (duplicate) SelectionManager.select(duplicate);
      });
    }
  } else {
    // create selection rect in tool overlays
  }

  function onPointerMove() {
    if ((element && SelectionManager.has(element.id)) || InputManager.keys.alt) {
      if (SelectionManager.size) {
        draggingOccurred = true;
        SelectionManager.forEach((entity) => {
          entity.translate(InputManager.scene.movement);
        });
        return;
      }
    }

    // if selection rect, update it
  }

  function onPointerUp(abort?: boolean) {
    SelectionManager.sync();
    // clear selection rect
    if (draggingOccurred && SelectionManager.size && !abort) {
      // apply translation of selected elements
    } else if (element && SelectionManager.has(element.id) && !elementIsAddedToSelection) {
      if (InputManager.keys.shift) {
        SelectionManager.deselect(element.id);
      } else {
        if (InputManager.button === BUTTONS.LEFT) SelectionManager.clear();
        SelectionManager.select(element);
      }
    }
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

export default onSelectPointerDown;
