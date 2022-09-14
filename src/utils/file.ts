export function fileDialog({ multiple, accept }: { multiple?: boolean; accept?: string[] }) {
  const input = document.createElement('input');

  if (multiple === true) input.setAttribute('multiple', '');
  if (accept) input.setAttribute('accept', accept.join(', '));
  input.setAttribute('type', 'file');

  input.style.display = 'none';
  input.setAttribute('id', 'hidden-file');
  document.body.appendChild(input);

  return new Promise((resolve) => {
    input.addEventListener('change', () => {
      resolve(input.files);
      document.body.removeChild(input);
    });

    const event = new MouseEvent('click', {});

    input.dispatchEvent(event);
  });
}
