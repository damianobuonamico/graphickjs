import { KEYS } from '@utils/keys';
import { vec2 } from '@math';
import Element from '../ecs/element';
import InputManager from '../input';
import { createVertices } from '../renderer/geometry';
import SceneManager from '../scene';

const onPolygonPointerDown = (tool: Tool) => {
  let size = vec2.create();

  let vertices = createVertices(tool, size);
  const element = new Element({ position: InputManager.scene.position, vertices });

  SceneManager.add(element);

  function onKey(e: KeyboardEvent) {
    if (e.key === KEYS.SHIFT || e.key === KEYS.ALT) {
      e.preventDefault();
      vertices = createVertices(tool, size, e.shiftKey, e.altKey);
      element.vertices = vertices;
    } else if (e.key === KEYS.ESCAPE) {
      SceneManager.remove(element);
    }
  }

  function onPointerMove() {
    size = InputManager.scene.delta;
    vertices = createVertices(tool, size, InputManager.keys.shift, InputManager.keys.alt);
    element.vertices = vertices;
  }

  function onPointerUp() {
    window.removeEventListener('keydown', onKey);
    window.removeEventListener('keyup', onKey);

    if (Math.abs(size[0]) < 1 && Math.abs(size[1]) < 1) SceneManager.remove(element);
  }

  window.addEventListener('keydown', onKey);
  window.addEventListener('keyup', onKey);

  return {
    onPointerMove,
    onPointerUp
  };
};

export default onPolygonPointerDown;
