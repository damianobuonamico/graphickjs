import { vec2 } from '@/math';
import { Transform } from '../ecs/components/transform';
import InputManager from '../input';
import { Renderer } from '../renderer';
import SelectionManager from '../selection';

const onRotatePointerDown = () => {
  const entity = InputManager.hover.entity;

  if (!entity || entity.type !== 'generichandle')
    return {
      onPointerMove: () => {},
      onPointerUp: () => {}
    };

  const entities: TransformableEntity[] = [];

  console.log(entity.id);

  SelectionManager.forEach((entity) => {
    if (
      (entity as TransformableEntity).transform &&
      (entity as TransformableEntity).transform instanceof Transform
    )
      entities.push(entity as TransformableEntity);
  });

  const box = SelectionManager.boundingBox;
  const origin = vec2.div(vec2.add(box[0], box[1]), 2);
  const start = vec2.sub(InputManager.scene.position, origin);

  let lastAngle = vec2.angle(start, vec2.sub(InputManager.scene.position, origin));

  function onPointerMove() {
    const angle = vec2.angle(start, vec2.sub(InputManager.scene.position, origin));

    entities.forEach((entity) => {
      const entityBox = entity.staticBoundingBox;
      const mid = vec2.div(vec2.add(entityBox[0], entityBox[1]), 2);
      const rotated = vec2.rotate(mid, origin, angle);

      entity.transform.clear();
      entity.transform.tempTranslate(vec2.sub(rotated, mid));
      entity.transform.tempRotate(angle);
    });

    SelectionManager.manipulator.transform.rotate(angle - (lastAngle || 0));
    lastAngle = angle;
  }

  function onPointerUp(abort?: boolean) {
    if (abort) {
      entities.forEach((entity) => entity.transform.clear());
    } else {
      console.log('apply');
      entities.forEach((entity) => entity.transform.apply());
    }
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

export default onRotatePointerDown;
