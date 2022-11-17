import { snap, vec2 } from '@/math';
import { isCompleteTransform } from '../ecs/components/transform';
import CommandHistory from '../history/history';
import InputManager from '../input';
import SelectionManager from '../selection';

const onRotatePointerDown = () => {
  const entity = InputManager.hover.entity;

  if (!entity || entity.type !== 'generichandle')
    return {
      onPointerMove: () => {},
      onPointerUp: () => {}
    };

  const transforms: TransformComponent[] = [];

  SelectionManager.forEach((entity) => {
    if (isCompleteTransform(entity.transform)) transforms.push(entity.transform);
  });

  const box = SelectionManager.unrotatedBoundingBox;
  const origin = vec2.mid(box[0], box[1]);
  const start = vec2.sub(InputManager.scene.position, origin);

  let lastAngle = vec2.angle(start, vec2.sub(InputManager.scene.position, origin));

  function onPointerMove() {
    let angle = vec2.angle(start, vec2.sub(InputManager.scene.position, origin));

    if (InputManager.keys.shift) angle = snap(angle);

    transforms.forEach((transform) => {
      const center = transform.staticCenter;

      transform.rotation.delta = angle;
      transform.position.delta = vec2.sub(vec2.rotate(center, origin, angle), center);
    });

    SelectionManager.manipulator.transform.rotate(angle - (lastAngle || 0));
    lastAngle = angle;
  }

  function onPointerUp(abort?: boolean) {
    if (abort) {
      CommandHistory.undo();
      CommandHistory.seal();
    } else {
      transforms.forEach((transform) => transform.apply());
    }

    SelectionManager.calculateRenderOverlay();
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

export default onRotatePointerDown;
