import { ZOOM_MAX, ZOOM_MIN, ZOOM_STEP } from '@/utils/constants';
import { clamp, round, vec2 } from '@math';
import InputManager from '../input';
import SceneManager from '../scene';

const onZoomPointerDown = () => {
  function onPointerMove() {
    const movement =
      Math.abs(InputManager.client.movement[0]) > Math.abs(InputManager.client.movement[1])
        ? InputManager.client.movement[0]
        : -InputManager.client.movement[1];

    const zoom = round(
      clamp(SceneManager.viewport.zoom * (1 + (movement * ZOOM_STEP) / 500), ZOOM_MIN, ZOOM_MAX),
      4
    );

    const delta = vec2.sub(
      SceneManager.clientToScene(InputManager.client.origin, {
        zoom
      }),
      SceneManager.clientToScene(InputManager.client.origin)
    );

    SceneManager.viewport.position = vec2.add(SceneManager.viewport.position, delta);
    SceneManager.viewport.zoom = zoom; 
  }

  return {
    onPointerMove
  };
};

export default onZoomPointerDown;
