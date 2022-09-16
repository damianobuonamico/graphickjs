function parseSVGText(node: SVGTextElement) {
  if (node instanceof Text) {
    if (node.data.trim() === '') return;
  }

  console.log('text', node);
}

export default parseSVGText;
