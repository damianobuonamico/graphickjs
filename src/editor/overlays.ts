import { Renderer } from './renderer';
import SceneManager from './scene';

interface Overlay {
  entity: Entity;
  static?: boolean;
  interactive?: boolean;
  condition?(): boolean;
}

class OverlayState {
  private m_overlays: Map<string, Overlay> = new Map();

  add(overlay: Overlay) {
    this.m_overlays.set(overlay.entity.id, overlay);
  }

  remove(id: string) {
    this.m_overlays.delete(id);
  }

  has(id: string) {
    return this.m_overlays.has(id);
  }

  get(id: string) {
    return this.m_overlays.get(id);
  }

  clear() {
    this.m_overlays.forEach((overlay, id) => {
      if (!overlay.static) this.m_overlays.delete(id);
    });
  }

  render() {
    Renderer.draw({
      operations: [
        { type: 'fillColor', data: ['#FFFFFF'] },
        { type: 'strokeColor', data: ['rgb(56, 195, 242'] },
        { type: 'strokeWidth', data: [1.5 / SceneManager.viewport.zoom] }
      ]
    });

    this.m_overlays.forEach((overlay) => {
      if (!overlay.condition || overlay.condition()) {
        Renderer.entity(overlay.entity, { inheritStrokeWidth: true });
      }
    });
  }

  public getEntityAt(position: vec2, lowerLevel = false, threshold: number = 0) {
    let toReturn: Entity | undefined = undefined;
    this.m_overlays.forEach((overlay) => {
      if (overlay.interactive && !toReturn) {
        const result = overlay.entity.getEntityAt(position, lowerLevel, threshold);
        if (result) toReturn = result;
      }
    });
    return toReturn;
  }
}

export default OverlayState;
