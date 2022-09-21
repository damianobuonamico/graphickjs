import HistoryManager from '../history';
import Fill from './components/fill';
import Stroke from './components/stroke';

class AssetsManager {
  private m_assets = {
    stroke: new Map<string, Stroke>(),
    fill: new Map<string, Fill>()
  };

  constructor() {}

  public set(asset: Stroke | Fill, skipRecordHistory: boolean = false) {
    let type: keyof typeof this.m_assets | null = null;

    if (asset instanceof Stroke) type = 'stroke';
    else if (asset instanceof Fill) type = 'fill';

    if (type) {
      if (skipRecordHistory) {
        this.m_assets[type!].set(asset.id, asset as any);
      } else {
        HistoryManager.record({
          fn: () => {
            this.m_assets[type!].set(asset.id, asset as any);
          },
          undo: () => {
            this.m_assets[type!].delete(asset.id);
          }
        });
      }
    }
  }

  public delete(id: string, type?: keyof typeof this.m_assets) {
    if (this.has(id, type)) {
      const asset = this.get(id);

      HistoryManager.record({
        fn: () => {
          if (type) {
            this.m_assets[type].delete(id);
          } else {
            const maps = Object.values(this.m_assets);

            for (let i = 0; i < maps.length; i++) {
              if (maps[i].has(id)) {
                maps[i].delete(id);
                break;
              }
            }
          }
        },
        undo: () => {
          this.set(asset);
        }
      });
    }
  }

  public has(id: string, type?: keyof typeof this.m_assets) {
    if (type) return this.m_assets[type].has(id);

    const maps = Object.values(this.m_assets);

    for (let i = 0; i < maps.length; i++) {
      if (maps[i].has(id)) return true;
    }

    return false;
  }

  public get<T extends Stroke | Fill>(id: string, type?: keyof typeof this.m_assets): T {
    if (type) {
      if (this.m_assets[type].has(id)) return this.m_assets[type].get(id) as T;
      return this.m_assets[type].values().next().value;
    }

    const maps = Object.values(this.m_assets);

    for (let i = 0; i < maps.length; i++) {
      if (maps[i].has(id)) return maps[i].get(id) as T;
    }

    return this.m_assets.stroke.values().next().value;
  }

  public load(object: AssetsObject) {
    object.stroke.forEach((obj) => this.set(new Stroke(obj)));
    object.fill.forEach((obj) => this.set(new Fill(obj)));
  }

  public toJSON() {
    return {
      stroke: Array.from(this.m_assets.stroke.values()).map((asset) => asset.toJSON(false)),
      fill: Array.from(this.m_assets.fill.values()).map((asset) => asset.toJSON(false))
    };
  }
}

export default AssetsManager;
