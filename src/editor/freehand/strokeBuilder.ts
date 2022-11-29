import { vec2 } from '@/math';

export function getFreehandGeometry(points: vec3[], position: vec2): [Float32Array, number[]] {
  const positions: number[] = [];
  const indices: number[] = [];

  if (points.length < 3) return [new Float32Array(0), []];

  let offset = 0;

  {
    const point: vec2 = [points[0][0], points[0][1]];
    const next: vec2 = [points[1][0], points[1][1]];
    const width = points[0][2] * 5;

    const v = vec2.sub(next, point);
    
    vec2.normalize(v, v);
    vec2.mulS(v, width, v);

    const na: vec2 = [-v[1], v[0]];
    const nc: vec2 = [v[1], -v[0]];

    positions.push(
      ...vec2.add(vec2.add(na, position, na), point, na),
      ...vec2.add(vec2.add(nc, position, nc), point, nc)
    );

    offset += 2;
  }

  for (let i = 1; i < points.length; ++i) {
    const prev: vec2 = [points[i - 1][0], points[i - 1][1]];
    const point: vec2 = [points[i][0], points[i][1]];
    const width = points[i][2] * 5;

    const v = vec2.sub(point, prev);

    vec2.normalize(v, v);
    vec2.mulS(v, width, v);

    const na: vec2 = [-v[1], v[0]];
    const nc: vec2 = [v[1], -v[0]];

    positions.push(
      ...vec2.add(vec2.add(na, position, na), point, na),
      ...vec2.add(vec2.add(nc, position, nc), point, nc)
    );
    indices.push(offset - 2, offset - 1, offset, offset - 1, offset, offset + 1);

    offset += 2;
  }

  return [Float32Array.from(positions), indices];
}
