import { isPointInBox } from '@/math';
import Eraser from '../ecs/entities/eraser';
import { isFreehand } from '../ecs/entities/freehand';
import InputManager from '../input';
import { Renderer } from '../renderer';
import SceneManager from '../scene';

const onEraserPointerDown = () => {
  const eraser = new Eraser({
    position: InputManager.scene.position
  });
  SceneManager.overlays.add({ entity: eraser });

  const radius = 20;

  function erase(position: vec2) {
    eraser.set(position, radius * (InputManager.pressure || 1));

    SceneManager.forEach((entity) => {
      if (!entity || !isFreehand(entity)) return;
      if (isPointInBox(position, entity.transform.boundingBox, radius)) {
        entity.erase(position, radius * (InputManager.pressure || 1));
      }
    });
  }

  erase(InputManager.scene.position);

  function onPointerMove() {
    erase(InputManager.scene.position);
  }
 
  return {
    onPointerMove
  };
};

export default onEraserPointerDown;
