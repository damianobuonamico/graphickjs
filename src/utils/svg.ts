import Element from '@/editor/ecs/element';
import Vertex from '@/editor/ecs/vertex';
import SceneManager from '@/editor/scene';
import SelectionManager from '@/editor/selection';
import { mat4, vec2, vec3 } from '@/math';

class SVGPather {
  private m_element: Element;

  constructor(element: Element) {
    this.m_element = element;
  }

  public get element() {
    return this.m_element;
  }

  private get position() {
    return this.m_element.position;
  }

  public close() {
    this.m_element.close();
  }

  public moveTo(position: vec2) {
    this.m_element.position = position;
    this.m_element.pushVertex(new Vertex({ position: vec2.create() }));
  }

  public lineTo(position: vec2) {
    this.m_element.pushVertex(new Vertex({ position: vec2.sub(position, this.position) }));
  }

  public quadraticBezierTo(cp: vec2, position: vec2) {
    const last = this.m_element.lastVertex;

    // Make control point if it differs from start and end points
    if (!vec2.equals(position, cp) && !vec2.equals(vec2.add(last.position, this.position), cp))
      last.setRight(vec2.sub(vec2.sub(cp, last.position), this.position));

    // Create end vertex
    this.m_element.pushVertex(new Vertex({ position: vec2.sub(position, this.position) }));
  }

  public smoothQuadraticBezierTo(position: vec2, clearSmooth = false) {
    const last = this.m_element.lastVertex;

    // Calculate control point through symmetry
    if (!clearSmooth && last.left && !vec2.equals(last.left.position, vec2.create()))
      last.setRight(vec2.neg(last.left.position));

    // Create end vertex
    this.m_element.pushVertex(new Vertex({ position: vec2.sub(position, this.position) }));
  }

  public cubicBezierTo(cp1: vec2, cp2: vec2, position: vec2) {
    const last = this.m_element.lastVertex;

    // Make first control point if it differs from start and end points
    if (!vec2.equals(position, cp1) && !vec2.equals(vec2.add(last.position, this.position), cp1))
      last.setRight(vec2.sub(vec2.sub(cp1, last.position), this.position));

    // Create end vertex
    const end = new Vertex({ position: vec2.sub(position, this.position) });
    this.m_element.pushVertex(end);

    // Make second control point if it differs from start and end points
    if (!vec2.equals(position, cp2) && !vec2.equals(vec2.add(last.position, this.position), cp2))
      end.setLeft(vec2.sub(vec2.sub(cp2, end.position), this.position));
  }

  public smoothCubicBezierTo(cp2: vec2, position: vec2, clearSmooth = false) {
    const last = this.m_element.lastVertex;

    // Calculate first control point through symmetry
    if (!clearSmooth && last.left && !vec2.equals(last.left.position, vec2.create()))
      last.setRight(vec2.neg(last.left.position));

    // Create end vertex
    const end = new Vertex({ position: vec2.sub(position, this.position) });
    this.m_element.pushVertex(end);

    // Make second control point if it differs from start and end points
    if (!vec2.equals(position, cp2) && !vec2.equals(vec2.add(last.position, this.position), cp2))
      end.setLeft(vec2.sub(vec2.sub(cp2, end.position), this.position));
  }
}

function arcToBeziers(angleStart: number, angleExtent: number) {
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

export function parseSVGPath(path: string, offset?: vec2) {
  const pather = new SVGPather(new Element({ position: vec2.create() }));

  // Normalize the path
  path = path
    .replace(/\s*([mlvhqcstazMLVHQCSTAZ])\s*/g, '\n$1 ')
    .replace(/,/g, ' ')
    .replace(/-/g, ' -')
    .replace(/ +/g, ' ');

  // Split the path in individual pathing instructions
  const strings = path.split('\n');

  // Setup element's position
  let translate = offset || vec2.create();
  let position = vec2.clone(translate);

  // Store last operation
  let lastOp = 'M';

  // Process each instruction
  for (let s = 1; s < strings.length; s++) {
    let instruction = strings[s].trim();
    let op = instruction.substring(0, 1);

    // Clear instructions from double dots
    let match = /\.\d*\.\d*/.exec(instruction);
    while (match) {
      for (let i = 1; i < match![0].length; i++) {
        if (match[0][i] != '.') continue;

        instruction =
          instruction.substring(0, match.index + i) + ' ' + instruction.substring(match.index + i);
      }

      match = /\.\d*\.\d*/.exec(instruction);
    }

    const data = instruction.length > 1 ? instruction.substring(2).trim().split(' ') : [];

    switch (op) {
      // Move instruction
      case 'M':
      case 'm': {
        if (op === 'm') vec2.add(position, [parseFloat(data[0]), parseFloat(data[1])], true);
        else position = vec2.add([parseFloat(data[0]), parseFloat(data[1])], translate);

        // Add a point only if the next operation is not another move operation or a close operation
        if (s < strings.length - 1) {
          let nextString = strings[s + 1].trim();
          let nextOp = nextString.substring(0, 1);

          if (!(nextOp === 'm' || nextOp === 'M' || nextOp === 'z' || nextOp === 'Z')) {
            if (s > 1) pather.close();

            pather.moveTo(position);
          }
        }

        break;
      }

      // Line instruction
      case 'L':
      case 'l': {
        for (let t = 0; t < data.length; t += 2) {
          if (op === 'l')
            vec2.add(position, [parseFloat(data[t + 0]), parseFloat(data[t + 1])], true);
          else position = vec2.add([parseFloat(data[t + 0]), parseFloat(data[t + 1])], translate);

          pather.lineTo(position);
        }

        break;
      }

      // Vertical line instruction
      case 'V':
      case 'v': {
        for (let t = 0; t < data.length; t++) {
          if (op === 'v') position[1] += parseFloat(data[t]);
          else position[1] = parseFloat(data[t]) + translate[1];

          pather.lineTo(position);
        }

        break;
      }

      // Horizontal line instruction
      case 'H':
      case 'h': {
        for (let t = 0; t < data.length; t++) {
          if (op === 'h') position[0] += parseFloat(data[t]);
          else position[0] = parseFloat(data[t]) + translate[0];

          pather.lineTo(position);
        }

        break;
      }

      // Quadratic bezier curve instruction
      case 'Q':
      case 'q': {
        for (let q = 0; q < data.length; q += 4) {
          // Calculate control point
          let center: vec2;
          if (op === 'q')
            center = vec2.add(position, [parseFloat(data[q]), parseFloat(data[q + 1])]);
          else center = vec2.add([parseFloat(data[q]), parseFloat(data[q + 1])], translate);

          // Calculate end point
          if (op === 'q')
            vec2.add(position, [parseFloat(data[q + 2]), parseFloat(data[q + 3])], true);
          else position = vec2.add([parseFloat(data[q + 2]), parseFloat(data[q + 3])], translate);

          pather.quadraticBezierTo(center, position);
        }

        break;
      }

      // Smooth quadratic bezier curve instruction
      case 'T':
      case 't': {
        for (let q = 0; q < data.length; q += 2) {
          // Calculate end point
          if (op === 't')
            vec2.add(position, [parseFloat(data[q + 2]), parseFloat(data[q + 3])], true);
          else position = vec2.add([parseFloat(data[q + 2]), parseFloat(data[q + 3])], translate);

          pather.smoothQuadraticBezierTo(position, lastOp.toLowerCase() === 'a');
        }

        break;
      }

      // Cubic bezier curve instruction
      case 'C':
      case 'c': {
        for (let c = 0; c < data.length; c += 6) {
          // Calculate first control point
          let cp1: vec2;
          if (op === 'c') cp1 = vec2.add(position, [parseFloat(data[c]), parseFloat(data[c + 1])]);
          else cp1 = vec2.add([parseFloat(data[c]), parseFloat(data[c + 1])], translate);

          // Calculate second control point
          let cp2: vec2;
          if (op === 'c')
            cp2 = vec2.add(position, [parseFloat(data[c + 2]), parseFloat(data[c + 3])]);
          else cp2 = vec2.add([parseFloat(data[c + 2]), parseFloat(data[c + 3])], translate);

          // Calculate end point
          if (op === 'c')
            vec2.add(position, [parseFloat(data[c + 4]), parseFloat(data[c + 5])], true);
          else position = vec2.add([parseFloat(data[c + 4]), parseFloat(data[c + 5])], translate);

          pather.cubicBezierTo(cp1, cp2, position);
        }

        break;
      }

      // Symmetric cubic bezier curve instruction
      case 'S':
      case 's': {
        for (let c = 0; c < data.length; c += 4) {
          // Calculate second control point
          let cp2: vec2;
          if (op === 's') cp2 = vec2.add(position, [parseFloat(data[c]), parseFloat(data[c + 1])]);
          else cp2 = vec2.add([parseFloat(data[c]), parseFloat(data[c + 1])], translate);

          // Create end vertex
          if (op === 's')
            vec2.add(position, [parseFloat(data[c + 2]), parseFloat(data[c + 3])], true);
          else position = vec2.add([parseFloat(data[c + 2]), parseFloat(data[c + 3])], translate);

          pather.smoothCubicBezierTo(cp2, position, lastOp.toLowerCase() === 'a');
        }

        break;
      }

      // Elliptic arc instruction
      case 'A':
      case 'a': {
        for (let c = 0; c < data.length; c += 7) {
          // Get last point
          const start = vec2.clone(position);

          // Calculate end point
          if (op === 'a')
            vec2.add(position, [parseFloat(data[c + 5]), parseFloat(data[c + 6])], true);
          else position = vec2.add([parseFloat(data[c + 5]), parseFloat(data[c + 6])], translate);

          // Get radius (absolute value)
          const radius = vec2.fromValues(
            Math.abs(parseFloat(data[c])),
            Math.abs(parseFloat(data[c + 1]))
          );

          // Get parameters
          const largeArcFlag = parseFloat(data[c + 3]);
          const sweepFlag = parseFloat(data[c + 4]);

          // If the endpoints is equal to the current position, ignore the instruction
          if (vec2.equals(start, position)) break;

          // Handle degenerate case
          if (radius[0] === 0 || radius[1] === 0) {
            // Create line segment
            pather.lineTo(position);
            break;
          }

          // Convert angle from degrees to radians
          const angle = (parseFloat(data[c + 2]) * Math.PI) / 180;
          const cosAngle = Math.cos(angle);
          const sinAngle = Math.sin(angle);

          // Midpoint of the line between the current and the end point
          const mid = vec2.div(vec2.sub(start, position), 2);

          // Rotated midpoint vector
          const rotated = [
            cosAngle * mid[0] + sinAngle * mid[1],
            -sinAngle * mid[0] + cosAngle * mid[1]
          ];

          let radiusSq = vec2.mul(radius, radius);
          const rotatedSq = vec2.mul(rotated, rotated);

          // If radii ar not large enough, scale them up
          const radiusCheck = rotatedSq[0] / radiusSq[0] + rotatedSq[1] / radiusSq[1];
          if (radiusCheck > 0.99999) {
            const radiusScale = Math.sqrt(radiusCheck) * 1.00001;

            vec2.mul(radius, radiusScale, true);
            radiusSq = vec2.mul(radius, radius);
          }

          // Computed the transformed center point
          let sign = largeArcFlag === sweepFlag ? -1 : 1;
          let sq = Math.max(
            (radiusSq[0] * radiusSq[1] - radiusSq[0] * rotatedSq[1] - radiusSq[1] * rotatedSq[0]) /
              (radiusSq[0] * rotatedSq[1] + radiusSq[1] * rotatedSq[0]),
            0
          );
          const coeff = sign * Math.sqrt(sq);
          const transformedCenter = [
            coeff * ((radius[0] * rotated[1]) / radius[1]),
            coeff * -((radius[1] * rotated[0]) / radius[0])
          ];

          // Compute the center poinr
          const s = vec2.div(vec2.add(start, position), 2);
          const center = [
            s[0] + (cosAngle * transformedCenter[0] - sinAngle * transformedCenter[1]),
            s[1] + (sinAngle * transformedCenter[0] + cosAngle * transformedCenter[1])
          ];

          const v1 = vec2.sub(start, center);
          const v2 = vec2.sub(position, center);

          let n: number, p: number;

          const TWO_PI = Math.PI * 2;

          let angleStart = vec2.angle(
            [1, 0],
            [
              (rotated[0] - transformedCenter[0]) / radius[0],
              (rotated[1] - transformedCenter[1]) / radius[1]
            ]
          );

          let angleExtent =
            vec2.angle(
              [
                (rotated[0] - transformedCenter[0]) / radius[0],
                (rotated[1] - transformedCenter[1]) / radius[1]
              ],
              [
                (-rotated[0] - transformedCenter[0]) / radius[0],
                (-rotated[1] - transformedCenter[1]) / radius[1]
              ]
            ) % TWO_PI;

          if (sweepFlag === 0 && angleExtent > 0) {
            angleExtent -= TWO_PI;
          } else if (sweepFlag === 1 && angleExtent < 0) {
            angleExtent += TWO_PI;
          }

          const beziers = arcToBeziers(angleStart, angleExtent);

          for (let i = 0; i < beziers.length; i++) {
            for (let t = 0; t < 3; t++) {
              vec2.mul(beziers[i][t], radius, true);
              // rotate(angle)
              vec2.add(beziers[i][t], center, true);
            }
          }

          // Replace last bezier's point with end point to avoid errors
          beziers[beziers.length - 1][2] = position;

          // Final step is to add the bezier curves to the path
          for (let i = 0; i < beziers.length; i++) {
            pather.cubicBezierTo(beziers[i][0], beziers[i][1], beziers[i][2]);
          }
        }

        break;
      }

      // Close element instruction
      case 'Z':
      case 'z': {
        pather.close();

        break;
      }
    }

    lastOp = op;
  }

  return pather.element;
}
