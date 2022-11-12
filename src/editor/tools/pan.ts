import { vec2 } from '@math';
import InputManager from '../input';
import SceneManager from '../scene';

const onPanPointerDown = () => {
  function onPointerMove() {
    vec2.add(
      SceneManager.viewport.position,
      InputManager.scene.movement,
      SceneManager.viewport.position
    );
  }

  return {
    onPointerMove
  };
};

export default onPanPointerDown;
