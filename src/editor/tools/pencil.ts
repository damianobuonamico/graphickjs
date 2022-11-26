import Freehand from '../ecs/entities/freehand';
import WobbleSmoother from '../freehand/wobbleSmoother';
import InputManager from '../input';
import SceneManager from '../scene';

const onPencilPointerDown = () => {
  const unprocessedFreehand = new Freehand({
    position: InputManager.scene.position,
    points: [[0, 0, InputManager.pressure || 0.5]]
  });

  SceneManager.add(unprocessedFreehand);

  WobbleSmoother.reset(
    { timeout: 40, speedFloor: 1.31, speedCeiling: 1.44 },
    [0, 0],
    InputManager.time
  );

  function onPointerMove() {
    unprocessedFreehand.add(InputManager.scene.delta, InputManager.pressure);
  }

  function onPointerUp() {
    onPointerMove();
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

export default onPencilPointerDown;
