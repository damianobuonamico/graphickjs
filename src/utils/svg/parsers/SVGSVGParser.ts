import SVGAttributesContainer from '../attributes';
import { parseSVGNodeChildren } from '../svg';

function parseSVGSVG(node: SVGSVGElement, attributes: SVGAttributesContainer) {
  // TODO: When scaling support is added, apply viewbox,width and height attributes
  // TODO: When groups support is added, process SVG node as a group and apply position (x, y) attributes
  // console.log('svg', node);

  return parseSVGNodeChildren(node, attributes);
}

export default parseSVGSVG;
