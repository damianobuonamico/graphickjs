interface ImageEntity extends TransformableEntity {
  selectable: true;

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
