function parseSVGText(node: SVGTextElement) {
  if (node instanceof Text) {
    if (node.data.trim() === '') return;
  }
}

export default parseSVGText;
