interface LayerCompositingComponentObject {
  opacity?: number;
}

interface LayerCompositingComponent {
  opacity: Value<number>;

  asObject(): LayerCompositingComponentObject;
}
