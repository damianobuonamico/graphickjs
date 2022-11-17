import Element from '@/editor/ecs/entities/element';
import Vertex from '@/editor/ecs/entities/vertex';
import { arcToBeziers } from '@/editor/renderer/geometry';
import { vec2 } from '@/math';
import { MATH_TWO_PI } from '@/utils/constants';
import SVGAttributesContainer from '../attributes';

interface SVGPathHistory {
  last: string | null;
  index: number;
  next: string | null;
}

class SVGPather {
  private m_element: Element;
  private m_cursor: vec2 = vec2.create();
  private m_lastQuadratic: vec2 | null = null;

  constructor(element: Element) {
    this.m_element = element;
  }

  public get element() {
    return this.m_element;
  }

  public get cursor() {
    return this.m_cursor;
  }

  private get position() {
    return this.m_element.transform.position;
  }

  private setCursor(position: vec2) {
    this.m_cursor = position;
    this.m_lastQuadratic = null;
  }

  public close() {
    this.m_element.close();
  }

  public moveTo(position: vec2) {
    this.m_element.transform.position.value = position;
    this.m_element.add(new Vertex({ position: vec2.create() }));
    this.setCursor(position);
  }

  public lineTo(position: vec2) {
    this.m_element.add(new Vertex({ position: vec2.sub(position, this.position.value) }));
    this.setCursor(position);
  }

  public quadraticBezierTo(cp: vec2, position: vec2) {
    const last = this.m_element.last;

    // Check if control point if it differs from start and end points
    if (
      vec2.equals(position, cp) ||
      vec2.equals(vec2.add(last.transform.position.value, this.position.value), cp)
    ) {
      this.lineTo(position);
      return;
    }

    // Raise curve order from quadratic to cubic
    const cp1 = vec2.add(this.m_cursor, vec2.mulS(vec2.sub(cp, this.m_cursor), 2 / 3));
    const cp2 = vec2.add(position, vec2.mulS(vec2.sub(cp, position), 2 / 3));

    this.cubicBezierTo(cp1, cp2, position);

    this.m_lastQuadratic = cp;
  }

  public smoothQuadraticBezierTo(position: vec2, clearSmooth = false) {
    const last = this.m_element.last;

    // Check if there is something to mirror
    if (
      clearSmooth ||
      !last.transform.left ||
      vec2.equals(last.transform.left.value, vec2.create())
    ) {
      this.lineTo(position);
      return;
    }

    const cp = this.m_lastQuadratic
      ? vec2.add(
          vec2.add(
            vec2.neg(
              vec2.sub(
                vec2.sub(this.m_lastQuadratic, last.transform.position.value),
                this.position.value
              )
            ),
            last.transform.position.value
          ),
          this.position.value
        )
      : vec2.add(
          vec2.add(vec2.neg(last.transform.left.value), last.transform.position.value),
          this.position.value
        );

    this.quadraticBezierTo(cp, position);
  }

  public cubicBezierTo(cp1: vec2, cp2: vec2, position: vec2) {
    const last = this.m_element.last;

    // Make first control point if it differs from start and end points
    if (
      !vec2.equals(position, cp1) &&
      !vec2.equals(vec2.add(last.transform.position.value, this.position.value), cp1)
    )
      last.transform.rightValue = vec2.sub(
        vec2.sub(cp1, last.transform.position.value),
        this.position.value
      );

    // Create end vertex
    const end = new Vertex({ position: vec2.sub(position, this.position.value) });
    this.m_element.add(end);

    // Make second control point if it differs from start and end points
    if (
      !vec2.equals(position, cp2) &&
      !vec2.equals(vec2.add(last.transform.position.value, this.position.value), cp2)
    )
      end.transform.leftValue = vec2.sub(
        vec2.sub(cp2, end.transform.position.value),
        this.position.value
      );

    this.setCursor(position);
  }

  public smoothCubicBezierTo(cp2: vec2, position: vec2, clearSmooth = false) {
    const last = this.m_element.last;

    // Calculate first control point through symmetry
    if (
      !clearSmooth &&
      last.transform.left &&
      !vec2.equals(last.transform.left.value, vec2.create())
    )
      last.transform.rightValue = vec2.neg(last.transform.left.value);

    // Create end vertex
    const end = new Vertex({ position: vec2.sub(position, this.position.value) });
    this.m_element.add(end);

    // Make second control point if it differs from start and end points
    if (
      !vec2.equals(position, cp2) &&
      !vec2.equals(vec2.add(last.transform.position.value, this.position.value), cp2)
    )
      end.transform.leftValue = vec2.sub(
        vec2.sub(cp2, end.transform.position.value),
        this.position.value
      );
    this.setCursor(position);
  }

  public arcTo(
    center: vec2,
    radius: vec2,
    angleStart: number,
    angleExtent: number,
    endPosition?: vec2
  ) {
    const beziers = arcToBeziers(angleStart, angleExtent);

    for (let i = 0, n = beziers.length; i < n; ++i) {
      for (let t = 0; t < 3; t++) {
        vec2.mul(beziers[i][t], radius, beziers[i][t]);
        // rotate(angle)
        vec2.add(beziers[i][t], center, beziers[i][t]);
      }
    }

    if (!beziers.length) return;

    // If provided, replace last bezier's point with end point to avoid errors
    if (endPosition) beziers[beziers.length - 1][2] = endPosition;

    this.m_cursor = beziers[beziers.length - 1][2];

    for (let i = 0, n = beziers.length; i < n; ++i) {
      this.cubicBezierTo(beziers[i][0], beziers[i][1], beziers[i][2]);
    }
  }
}

function parseSVGPath(node: SVGPathElement, attributes: SVGAttributesContainer) {
  attributes.get(node);

  const d = node.getAttribute('d');

  // If path data is not provided, discard the node
  if (!d) return null;

  // Normalize and split path data in single instructions
  const instructions = d
    .replace(/\s*([mlvhqcstazMLVHQCSTAZ])\s*/g, '\n$1 ')
    .replace(/,/g, ' ')
    .replace(/-/g, ' -')
    .replace(/ +/g, ' ')
    .split('\n');

  const pather = new SVGPather(
    new Element({
      position: vec2.create(),
      stroke: attributes.stroke,
      fill: attributes.fill
    })
  );

  const history: SVGPathHistory = {
    last: null,
    index: 0,
    next: null
  };

  // Process each instruction
  for (let i = 0, n = instructions.length; i < n; ++i) {
    let instruction = instructions[i].trim();

    history.index = i;
    history.next = i < instructions.length - 1 ? instructions[i + 1].trim()[0] : null;

    // Filter empty instructions
    if (instruction.length === 0) continue;

    // Clear instructions from double dots
    let match = /\.\d*\.\d*/.exec(instruction);
    while (match) {
      for (let i = 1, n = match![0].length; i < n; ++i) {
        if (match[0][i] != '.') continue;

        instruction =
          instruction.substring(0, match.index + i) + ' ' + instruction.substring(match.index + i);
      }

      match = /\.\d*\.\d*/.exec(instruction);
    }

    const op = instruction[0];
    const data = instruction.length > 1 ? instruction.substring(2).trim().split(' ') : [];

    (SVGPathDataParsers[op.toLowerCase()] || SVGPathDataParsers.default)(pather, op, data, history);

    history.last = op;
  }

  pather.element.transform.apply();
  return pather.element;
}

const SVGPathDataParsers: {
  [key: string]: (pather: SVGPather, op: string, data: string[], history: SVGPathHistory) => void;
} = {
  m: parseSVGPathM,
  z: parseSVGPathZ,
  l: parseSVGPathL,
  h: parseSVGPathH,
  v: parseSVGPathV,
  c: parseSVGPathC,
  s: parseSVGPathS,
  q: parseSVGPathQ,
  t: parseSVGPathT,
  a: parseSVGPathA,
  default: () => {}
};

function parseSVGPathM(pather: SVGPather, op: string, data: string[], history: SVGPathHistory) {
  for (let i = 0, n = data.length; i < n; i += 2) {
    const position: vec2 = [parseFloat(data[i + 0]), parseFloat(data[i + 1])];
    if (op === 'm') vec2.add(position, pather.cursor, position);

    // Add a point only if the next operation is not another move operation or a close operation
    if (history.next) {
      const nextOp = history.next.toLowerCase();

      if (!(nextOp === 'm' || nextOp === 'z')) {
        if (history.index > 1) pather.close();

        pather.moveTo(position);
      }
    }
  }
}

function parseSVGPathZ(pather: SVGPather) {
  pather.close();
}

function parseSVGPathL(pather: SVGPather, op: string, data: string[]) {
  for (let i = 0, n = data.length; i < n; i += 2) {
    const position: vec2 = [parseFloat(data[i + 0]), parseFloat(data[i + 1])];
    if (op === 'l') vec2.add(position, pather.cursor, position);

    pather.lineTo(position);
  }
}

function parseSVGPathH(pather: SVGPather, op: string, data: string[]) {
  for (let i = 0, n = data.length; i < n; ++i) {
    const position: vec2 = [parseFloat(data[i]), 0];
    if (op === 'h') vec2.add(position, pather.cursor, position);
    else position[1] = pather.cursor[1];

    pather.lineTo(position);
  }
}

function parseSVGPathV(pather: SVGPather, op: string, data: string[]) {
  for (let i = 0, n = data.length; i < n; ++i) {
    const position: vec2 = [0, parseFloat(data[i])];
    if (op === 'v') vec2.add(position, pather.cursor, position);
    else position[0] = pather.cursor[0];

    pather.lineTo(position);
  }
}

function parseSVGPathC(pather: SVGPather, op: string, data: string[]) {
  for (let i = 0, n = data.length; i < n; i += 6) {
    const cp1: vec2 = [parseFloat(data[i + 0]), parseFloat(data[i + 1])];
    const cp2: vec2 = [parseFloat(data[i + 2]), parseFloat(data[i + 3])];
    const position: vec2 = [parseFloat(data[i + 4]), parseFloat(data[i + 5])];

    if (op === 'c') {
      vec2.add(cp1, pather.cursor, cp1);
      vec2.add(cp2, pather.cursor, cp2);
      vec2.add(position, pather.cursor, position);
    }

    pather.cubicBezierTo(cp1, cp2, position);
  }
}

function parseSVGPathS(pather: SVGPather, op: string, data: string[], history: SVGPathHistory) {
  for (let i = 0, n = data.length; i < n; i += 4) {
    const cp2: vec2 = [parseFloat(data[i + 0]), parseFloat(data[i + 1])];
    const position: vec2 = [parseFloat(data[i + 2]), parseFloat(data[i + 3])];

    if (op === 's') {
      vec2.add(cp2, pather.cursor, cp2);
      vec2.add(position, pather.cursor, position);
    }

    pather.smoothCubicBezierTo(cp2, position, history.last?.toLowerCase() === 'a');
  }
}

function parseSVGPathQ(pather: SVGPather, op: string, data: string[]) {
  for (let i = 0, n = data.length; i < n; i += 4) {
    const cp: vec2 = [parseFloat(data[i + 0]), parseFloat(data[i + 1])];
    const position: vec2 = [parseFloat(data[i + 2]), parseFloat(data[i + 3])];

    if (op === 'q') {
      vec2.add(cp, pather.cursor, cp);
      vec2.add(position, pather.cursor, position);
    }

    pather.quadraticBezierTo(cp, position);
  }
}

function parseSVGPathT(pather: SVGPather, op: string, data: string[], history: SVGPathHistory) {
  for (let i = 0, n = data.length; i < n; i += 2) {
    const position: vec2 = [parseFloat(data[i + 0]), parseFloat(data[i + 1])];
    if (op === 't') vec2.add(position, pather.cursor, position);

    pather.smoothQuadraticBezierTo(position, history.last?.toLowerCase() === 'a');
  }
}

function parseSVGPathA(pather: SVGPather, op: string, data: string[]) {
  for (let i = 0, n = data.length; i < n; i += 7) {
    const position: vec2 = [parseFloat(data[i + 5]), parseFloat(data[i + 6])];
    if (op === 'a') vec2.add(position, pather.cursor, position);

    // Get absolute value of the radius
    const radius = vec2.abs([parseFloat(data[i + 0]), parseFloat(data[i + 1])]);

    // Parameters to determine which one of the four ellipses to select
    const largeArcFlag = parseFloat(data[i + 3]);
    const sweepFlag = parseFloat(data[i + 4]);

    // If the endpoints is equal to the current position, ignore the instruction
    if (vec2.equals(pather.cursor, position)) break;

    // Handle degenerate case
    if (radius[0] === 0 || radius[1] === 0) {
      // Create line segment
      pather.lineTo(position);
      break;
    }

    // Convert angle from degrees to radians
    const angle = (parseFloat(data[i + 2]) * Math.PI) / 180;
    const cosAngle = Math.cos(angle);
    const sinAngle = Math.sin(angle);

    // Midpoint of the line between the current position and the end point
    const mid = vec2.divS(vec2.sub(pather.cursor, position), 2);

    // Rotated midpoint vector
    const rotated: vec2 = [
      cosAngle * mid[0] + sinAngle * mid[1],
      -sinAngle * mid[0] + cosAngle * mid[1]
    ];

    let radiusSq = vec2.mul(radius, radius);
    const rotatedSq = vec2.mul(rotated, rotated);

    // If radius is not large enough, scale them up
    const radiusCheck = rotatedSq[0] / radiusSq[0] + rotatedSq[1] / radiusSq[1];
    if (radiusCheck > 0.99999) {
      const radiusScale = Math.sqrt(radiusCheck) * 1.00001;

      vec2.mulS(radius, radiusScale, radius);
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
    const s = vec2.mid(pather.cursor, position);
    const center: vec2 = [
      s[0] + (cosAngle * transformedCenter[0] - sinAngle * transformedCenter[1]),
      s[1] + (sinAngle * transformedCenter[0] + cosAngle * transformedCenter[1])
    ];

    // Vector between center and start point
    const v: vec2 = [
      (rotated[0] - transformedCenter[0]) / radius[0],
      (rotated[1] - transformedCenter[1]) / radius[1]
    ];

    const angleStart = vec2.angle([1, 0], v);

    let angleExtent =
      vec2.angle(v, [
        (-rotated[0] - transformedCenter[0]) / radius[0],
        (-rotated[1] - transformedCenter[1]) / radius[1]
      ]) % MATH_TWO_PI;

    if (sweepFlag === 0 && angleExtent > 0) {
      angleExtent -= MATH_TWO_PI;
    } else if (sweepFlag === 1 && angleExtent < 0) {
      angleExtent += MATH_TWO_PI;
    }

    pather.arcTo(center, radius, angleStart, angleExtent, position);
  }
}

export default parseSVGPath;
