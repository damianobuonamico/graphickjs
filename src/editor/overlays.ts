import { Renderer } from './renderer';

interface Overlay {
  entity: Entity;
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

  render() {
    Renderer.draw({
      operations: [
        { type: 'fillcolor', data: [1.0, 1.0, 1.0, 1.0] },
        { type: 'strokecolor', data: [49 / 255, 239 / 255, 284 / 255, 1] }
      ]
    });

    this.m_overlays.forEach((overlay) => {
      if (!overlay.condition || overlay.condition()) {
        Renderer.entity(overlay.entity);
      }
    });
  }

  public getEntityAt(position: vec2, lowerLevel = false, threshold: number = 0) {
    let toReturn: Entity | undefined = undefined;
    this.m_overlays.forEach((overlay) => {
      if (!toReturn) {
        const result = overlay.entity.getEntityAt(position, lowerLevel, threshold);
        if (result) toReturn = result;
      }
    });
    return toReturn;
  }
}

export default OverlayState;
