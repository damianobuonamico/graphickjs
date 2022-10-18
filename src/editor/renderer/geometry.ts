import { GEOMETRY_CIRCLE_RATIO } from '@utils/constants';
import { vec2 } from '@math';
import Vertex from '../ecs/entities/vertex';

export function createVertices(
  type: 'rectangle' | 'ellipse',
  size: vec2 = [0, 0],
  perfect: boolean = false,
  centered: boolean = false
) {
  size = [size[0], perfect ? (Math.sign(size[1]) || 1) * Math.abs(size[0]) : size[1]];
  size = centered ? vec2.mulS(size, 2) : size;
  switch (type) {
    case 'rectangle':
      return createRectangleVertices(size, centered);
    case 'ellipse':
      return createEllipseVertices(size, centered);
    default:
      return [];
  }
}

function createRectangleVertices(size: vec2, centered: boolean) {
  const half = vec2.mulS(size, 0.5);
  const translate: vec2 = centered ? half : [0, 0];

  return [
    new Vertex({ position: vec2.sub([0, 0], translate) }),
    new Vertex({ position: vec2.sub([size[0], 0], translate) }),
    new Vertex({ position: vec2.sub(size, translate) }),
    new Vertex({ position: vec2.sub([0, size[1]], translate) })
  ];
}

function createEllipseVertices(size: vec2, centered: boolean): Vertex[] {
  const half = vec2.div(size, [2, 2]);
  const translate: vec2 = centered ? half : [0, 0];
  const handle = vec2.mul(half, [GEOMETRY_CIRCLE_RATIO, GEOMETRY_CIRCLE_RATIO]);
  return [
    new Vertex({
      position: vec2.sub([half[0], 0], translate),
      right: [handle[0], 0],
      left: [-handle[0], 0]
    }),
    new Vertex({
      position: vec2.sub([size[0], half[1]], translate),
      right: [0, handle[1]],
      left: [0, -handle[1]]
    }),
    new Vertex({
      position: vec2.sub([half[0], size[1]], translate),
      right: [-handle[0], 0],
      left: [handle[0], 0]
    }),
    new Vertex({
      position: vec2.sub([0, half[1]], translate),
      right: [0, -handle[1]],
      left: [0, handle[1]]
    })
  ];
}

export function arcToBeziers(angleStart: number, angleExtent: number) {
  const numSegments = Math.floor((Math.abs(angleExtent) * 2.0) / Math.PI); // (angleExtent / 90deg)

  const angleIncrement = angleExtent / numSegments;

  // The length of each control point vector is given by the following formula.
  const controlLength =
    ((4.0 / 3.0) * Math.sin(angleIncrement / 2.0)) / (1.0 + Math.cos(angleIncrement / 2.0));

  const beziers: [vec2, vec2, vec2][] = [];

  for (let i = 0; i < numSegments; i++) {
    let angle = angleStart + i * angleIncrement;

    // Calculate the control vector at this angle
    let d = [Math.cos(angle), Math.sin(angle)];

    // First control point
    const bezier = [[d[0] - controlLength * d[1], d[1] + controlLength * d[0]]];

    // Second control point;
    angle += angleIncrement;
    d = [Math.cos(angle), Math.sin(angle)];

    bezier.push([d[0] + controlLength * d[1], d[1] - controlLength * d[0]]);

    // Endpoint of bezier
    bezier.push([d[0], d[1]]);

    beziers.push(bezier as [vec2, vec2, vec2]);
  }

  return beziers;
}
