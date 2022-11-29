export function getFreehandGeometry(
  points: vec3[],
  zoom: number,
  position: vec2
): [Float32Array, number[]] {
  const positions: number[] = [];
  const indices: number[] = [];

  let offset = 0;

  for (let i = 0; i < points.length; ++i) {
    positions.push(
      points[i][0] + position[0] - 5 / zoom,
      points[i][1] + position[1] - 5 / zoom,
      points[i][0] + position[0] + 5 / zoom,
      points[i][1] + position[1] - 5 / zoom,
      points[i][0] + position[0] + 5 / zoom,
      points[i][1] + position[1] + 5 / zoom,
      points[i][0] + position[0] - 5 / zoom,
      points[i][1] + position[1] + 5 / zoom
    );
    indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
    offset += 4;
  }

  // positions.push(
  //   position[0] - 500 / zoom,
  //   position[1] - 500 / zoom,
  //   position[0] + 500 / zoom,
  //   position[1] - 500 / zoom,
  //   position[0] + 500 / zoom,
  //   position[1] + 500 / zoom,
  //   position[0] - 500 / zoom,
  //   position[1] + 500 / zoom
  // );
  // indices.push(0, 1, 2, 2, 3, 0);

  return [Float32Array.from(positions), indices];
}
