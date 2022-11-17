interface ImageEntity extends Entity {
  readonly type: 'image';
  readonly selectable: true;
  readonly transform: RectTransformComponent;

  size: vec2;
  source: HTMLImageElement;
}

interface ImageOptions {
  id?: string;
  source: string;
  position?: vec2;
  transform?: TransformComponentObject;
  size?: vec2;
}

interface ImageObject extends GenericEntityObject {
  source: string;
  position?: vec2;
  transform?: TransformComponentObject;
  size: vec2;
}
