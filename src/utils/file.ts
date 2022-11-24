export function fileDialog({ multiple, accept }: { multiple?: boolean; accept?: string[] }) {
  // Create and setup input element
  const input = document.createElement('input');

  if (multiple === true) input.setAttribute('multiple', '');
  if (accept) input.setAttribute('accept', accept.join(', '));
  input.setAttribute('type', 'file');

  // Hide input element
  input.style.display = 'none';
  input.setAttribute('id', 'hidden-file');
  document.body.appendChild(input);

  return new Promise<FileList>((resolve) => {
    input.addEventListener('change', () => {
      if (input.files) resolve(input.files);

      // Cleanup input element
      document.body.removeChild(input);
    });

    // Click on the input element to open file dialog
    const event = new MouseEvent('click', {});
    input.dispatchEvent(event);
  });
}

export function download(
  data: BlobPart,
  name: string = 'Untitled',
  format: string = 'application/json'
) {
  const file = new Blob([data], { type: format });

  const a = document.createElement('a'),
    url = URL.createObjectURL(file);

  a.href = url;
  a.download = name;
  document.body.appendChild(a);
  a.click();

  setTimeout(function () {
    document.body.removeChild(a);
    window.URL.revokeObjectURL(url);
  }, 0);
}
