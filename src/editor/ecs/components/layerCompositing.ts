import { FloatValue } from '@/editor/history/value';

class LayerCompositing implements LayerCompositingComponent {
  readonly opacity: Value<number>;

  constructor(opacity: number = 1) {
    this.opacity = new FloatValue(opacity);
  }

  asObject(): LayerCompositingComponentObject {
    const opacity = this.opacity.value;

    if (opacity === 1) return {};

    return { opacity };
  }
}

export default LayerCompositing;
