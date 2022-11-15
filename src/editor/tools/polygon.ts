import { KEYS } from '@utils/keys';
import { vec2 } from '@math';
import Element from '../ecs/entities/element';
import InputManager from '../input';
import { createVertices } from '../renderer/geometry';
import SceneManager from '../scene';
import HistoryManager from '../history/history';
import { nanoid } from 'nanoid';

const onPolygonPointerDown = (tool: Tool) => {
  let size = vec2.create();

  let vertices = createVertices(tool as 'rectangle' | 'ellipse', size);
  const element = new Element({
    position: InputManager.scene.position,
    vertices,
    closed: true,
    stroke: { color: [0, 0, 0, 1] },
    fill: { color: [1, 1, 1, 0] }
  });
  SceneManager.add(element);

  function onKey(e: KeyboardEvent) {
    if (e.key === KEYS.SHIFT || e.key === KEYS.ALT) {
      e.preventDefault();
      vertices = createVertices(tool as 'rectangle' | 'ellipse', size, e.shiftKey, e.altKey);
      element.vertices = vertices;
    } else if (e.key === KEYS.ESCAPE) {
      SceneManager.remove(element, true);
      HistoryManager.pop();
    }
  }

  function onPointerMove() {
    size = InputManager.scene.delta;
    vertices = createVertices(
      tool as 'rectangle' | 'ellipse',
      size,
      InputManager.keys.shift,
      InputManager.keys.alt
    );
    element.vertices = vertices;
  }

  function onPointerUp() {
    if (Math.abs(size[0]) < 1 && Math.abs(size[1]) < 1) SceneManager.remove(element);
  }

  return {
    onPointerMove,
    onPointerUp,
    onKey
  };
};

export default onPolygonPointerDown;
