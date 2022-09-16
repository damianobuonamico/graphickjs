import SVGAttributesContainer from '../attributes';
import { parseSVGNodeChildren } from '../svg';

function parseSVGG(node: SVGGElement, attributes: SVGAttributesContainer) {
  console.log('g', node);

  if (node.childNodes.length) {
    attributes.get(node);
    return parseSVGNodeChildren(node, attributes);
  }
}

export default parseSVGG;
