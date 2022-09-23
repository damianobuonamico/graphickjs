import InputManager from '../input';

const onScalePointerDown = () => {
  const entity = InputManager.hover.entity;

  if (!entity || entity.type !== 'generichandle')
    return {
      onPointerMove: () => {},
      onPointerUp: () => {}
    };

  console.log(entity.id);

  function onPointerMove() {}

  function onPointerUp(abort?: boolean) {}

  return {
    onPointerMove,
    onPointerUp
  };
};

export default onScalePointerDown;
