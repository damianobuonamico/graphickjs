import { vec2 } from '@math';

export function createVertices(
  type: string,
  size: vec2 = [0, 0],
  perfect: boolean = false,
  centered: boolean = false
) {
  size = [
    size[0],
    perfect ? (Math.sign(size[1]) || 1) * Math.abs(size[0]) : size[1]
  ];
  size = centered ? vec2.mul(size, 2) : size;
  switch (type) {
    case 'rectangle':
      return createRectangleVertices(size, centered);
    default:
      return [];
  }
}

function createRectangleVertices(size: vec2, centered: boolean) {
  const half = vec2.mul(size, 0.5);
  const translate: vec2 = centered ? half : [0, 0];
  return [
    vec2.sub([0, 0], translate),
    vec2.sub([size[0], 0], translate),
    vec2.sub(size, translate),
    vec2.sub([0, size[1]], translate)
  ];
}
