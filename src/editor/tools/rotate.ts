const onRotatePointerDown = () => {
  console.log('rotate');

  function onPointerMove() {}

  function onPointerUp(abort?: boolean) {}

  return {
    onPointerMove,
    onPointerUp
  };
};

export default onRotatePointerDown;
