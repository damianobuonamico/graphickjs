import SVGAttributesContainer from './attributes';
import parseSVGG from './parsers/SVGGParser';
import parseSVGPath from './parsers/SVGPathParser';
import parseSVGSVG from './parsers/SVGSVGParser';
import parseSVGText from './parsers/SVGTextParser';

export function parseSVG(svg: string) {
  // Convert string to SVG tree
  const parser = new DOMParser();
  const tree = parser.parseFromString(svg.trim(), 'image/svg+xml');
  const attributes = new SVGAttributesContainer();

  return parseSVGNode(tree, attributes);
}

const SVGParsers: {
  [key: string]: (node: any, attributes: SVGAttributesContainer) => Entity[] | Entity | null | void;
} = {
  svg: parseSVGSVG,
  '#document': importSVGDocument,
  '#text': parseSVGText,
  path: parseSVGPath,
  g: parseSVGG,
  default: parseSVGG
};

export function parseSVGNode(node: Node, attributes: SVGAttributesContainer) {
  const type = node.nodeName.toLowerCase();

  // Parse unknown nodes as <g>
  return (SVGParsers[type] || SVGParsers.default)(node, attributes);
}

export function parseSVGNodeChildren(node: Node, attributes: SVGAttributesContainer) {
  // Traverse attribute tree
  attributes.level++;

  const entities: Entity[] = [];

  node.childNodes.forEach((child) => {
    const entity = parseSVGNode(child, attributes);
    if (entity) {
      if (Array.isArray(entity)) entities.push(...entity);
      else entities.push(entity);
    }
  });

  attributes.level--;

  return entities;
}

function importSVGDocument(node: Node, attributes: SVGAttributesContainer) {
  // TODO: Group imported svg
  return parseSVGNodeChildren(node, attributes);
}
