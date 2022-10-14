import { vec2 } from '@/math';
import { KEYS } from '@/utils/keys';
import InputManager from '../input';
import SelectionManager from '../selection';

const onScalePointerDown = () => {
  const entity = InputManager.hover.entity;

  if (!entity || entity.type !== 'generichandle')
    return {
      onPointerMove: () => {},
      onPointerUp: () => {}
    };

  const box = SelectionManager.boundingBox;
  const mid = vec2.div(vec2.add(box[0], box[1]), 2);

  const rotated = [
    vec2.rotate(box[0], mid, SelectionManager.angle || 0),
    vec2.rotate([box[1][0], box[0][1]], mid, SelectionManager.angle || 0),
    vec2.rotate(box[1], mid, SelectionManager.angle || 0),
    vec2.rotate([box[0][0], box[1][1]], mid, SelectionManager.angle || 0)
  ];

  const midpoints = [
    vec2.div(vec2.add(rotated[0], rotated[1]), 2),
    vec2.div(vec2.add(rotated[1], rotated[2]), 2),
    vec2.div(vec2.add(rotated[2], rotated[3]), 2),
    vec2.div(vec2.add(rotated[3], rotated[0]), 2)
  ];

  let center = vec2.clone(mid);

  const size = vec2.sub(box[1], box[0]);
  let axial = false;

  switch (entity.id) {
    case 'scale-n':
      center = midpoints[2];
      axial = true;
      break;
    case 'scale-s':
      center = midpoints[0];
      axial = true;
      break;
    case 'scale-e':
      center = midpoints[3];
      axial = true;
      break;
    case 'scale-w':
      center = midpoints[1];
      axial = true;
      break;
    case 'scale-nw':
      center = rotated[2];
      break;
    case 'scale-ne':
      center = rotated[3];
      break;
    case 'scale-sw':
      center = rotated[1];
      break;
    case 'scale-se':
      center = rotated[0];
      break;
  }

  const dist = vec2.sub(
    vec2.rotate(InputManager.scene.position, mid, -(SelectionManager.angle || 0)),
    vec2.rotate(center, mid, -(SelectionManager.angle || 0))
  );

  function onKey(e: KeyboardEvent) {
    if (e.key === KEYS.SHIFT) {
      onPointerMove();
    }
  }

  function onPointerMove() {
    const magnitude = vec2.div(
      vec2.sub(
        vec2.rotate(InputManager.scene.position, mid, -(SelectionManager.angle || 0)),
        vec2.rotate(center, mid, -(SelectionManager.angle || 0))
      ),
      dist
    );

    switch (entity!.id) {
      case 'scale-n':
      case 'scale-s':
        magnitude[0] = 1;
        break;
      case 'scale-e':
      case 'scale-w':
        magnitude[1] = 1;
        break;
    }

    if (InputManager.keys.shift) {
      if (axial) {
        if (magnitude[0] === 1) {
          magnitude[0] = magnitude[1];
        } else if (magnitude[1] === 1) {
          magnitude[1] = magnitude[0];
        }
      } else {
        if (magnitude[0] > magnitude[1]) {
          magnitude[0] = magnitude[1];
        } else {
          magnitude[1] = magnitude[0];
        }
      }
    }

    SelectionManager.forEach((entity) => {
      const box1 = (entity as TransformableEntity).unrotatedBoundingBox;
      const mid1 = vec2.div(vec2.add(box1[0], box1[1]), 2);
      (entity as TransformableEntity).transform.origin = vec2.sub(
        vec2.rotate(center, mid1, -(entity as TransformableEntity).transform.rotation),
        (entity as TransformableEntity).transform.position
      );

      (entity as TransformableEntity).transform.tempScale(magnitude);

      const box2 = (entity as TransformableEntity).unrotatedBoundingBox;
      const mid2 = vec2.div(vec2.add(box2[0], box2[1]), 2);

      (entity as TransformableEntity).transform.tempTranslate(
        vec2.sub(
          vec2.rotate([0, 0], mid1, (entity as TransformableEntity).transform.rotation),
          vec2.rotate([0, 0], mid2, (entity as TransformableEntity).transform.rotation)
        )
      );
    });

    SelectionManager.calculateRenderOverlay();
  }

  function onPointerUp(abort?: boolean) {
    if (abort) {
      SelectionManager.forEach((entity) => {
        (entity as MovableEntity).transform.clear();
      });
    } else {
      SelectionManager.forEach((entity) => {
        (entity as MovableEntity).transform.apply();
      });
    }
  }

  return {
    onKey,
    onPointerMove,
    onPointerUp
  };
};

export default onScalePointerDown;
