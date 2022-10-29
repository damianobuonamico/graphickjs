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

  const box = SelectionManager.unrotatedBoundingBox;
  const mid = vec2.mid(box[0], box[1]);

  const angle = SelectionManager.angle;

  const rotated = [
    vec2.rotate(box[0], mid, angle || 0),
    vec2.rotate([box[1][0], box[0][1]], mid, angle || 0),
    vec2.rotate(box[1], mid, angle || 0),
    vec2.rotate([box[0][0], box[1][1]], mid, angle || 0)
  ];

  const midpoints = [
    vec2.mid(rotated[0], rotated[1]),
    vec2.mid(rotated[1], rotated[2]),
    vec2.mid(rotated[2], rotated[3]),
    vec2.mid(rotated[3], rotated[0])
  ];

  let center = vec2.clone(mid);

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
    vec2.rotate(InputManager.scene.position, mid, -(angle || 0)),
    vec2.rotate(center, mid, -(angle || 0))
  );

  function onKey(e: KeyboardEvent) {
    if (e.key === KEYS.SHIFT || e.key === KEYS.ALT) {
      e.preventDefault();
      onPointerMove();
    }
  }

  let backup = center;

  // TODO: Fix rotated image + element scaling
  function onPointerMove() {
    if (InputManager.keys.alt) center = mid;
    else center = backup;

    const magnitude = vec2.div(
      vec2.sub(
        vec2.rotate(InputManager.scene.position, mid, -(angle || 0)),
        vec2.rotate(center, mid, -(angle || 0))
      ),
      dist
    );

    if (InputManager.keys.alt) {
      vec2.mulS(magnitude, 2, magnitude);
    }

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
      if ((entity.transform as RectTransformComponent).tempScale) {
        (entity.transform as RectTransformComponent).origin = center;
        (entity.transform as RectTransformComponent).tempScale(magnitude, angle === null);
      }
    });

    SelectionManager.calculateRenderOverlay();
  }

  function onPointerUp(abort?: boolean) {
    if (abort) {
      SelectionManager.forEach((entity) => {
        (entity.transform as TransformComponent).clear();
      });
    } else {
      SelectionManager.forEach((entity) => {
        (entity.transform as TransformComponent).apply();
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
