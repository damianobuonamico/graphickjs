import { vec2 } from '@math';
import InputManager from '../input';
import SceneManager from '../scene';

const onPanPointerDown = () => {
  function onPointerMove() {
    SceneManager.viewport.position = vec2.add(
      SceneManager.viewport.position,
      InputManager.client.movement
    );
  }

  return {
    onPointerMove
  };
};

export default onPanPointerDown;
