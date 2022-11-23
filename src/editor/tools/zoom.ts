import { ZOOM_STEP } from '@/utils/constants';
import InputManager from '../input';
import SceneManager from '../scene';

const onZoomPointerDown = () => {
  function onPointerMove() {
    const movement =
      Math.abs(InputManager.client.movement[0]) > Math.abs(InputManager.client.movement[1])
        ? InputManager.client.movement[0]
        : -InputManager.client.movement[1];

    SceneManager.viewport.zoom = [
      SceneManager.viewport.zoom * (1 + (movement * ZOOM_STEP) / 500),
      InputManager.client.origin
    ];
  }

  return {
    onPointerMove
  };
};

export default onZoomPointerDown;
