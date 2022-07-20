import { Component, createSignal } from 'solid-js';
import FileMenu from '@menu/FileMenu';

const TitleBar: Component = () => {
  const [checked, setChecked] = createSignal(true);
  return (
    <div class="bg-primary-800 h-8 w-full flex items-center border-primary-600 border-b">
      <FileMenu
        items={[
          {
            label: 'File',
            submenu: [
              { label: 'New...' },
              { label: 'New From Template...' },
              { label: 'Open...' },
              {
                label: 'Open Recent Files From HD',
                submenu: [
                  { label: 'foo.gk' },
                  { label: 'bar.gk' },
                  { label: 'Older', submenu: [{ label: 'old_foo.gk' }, { label: 'old_bar.gk' }] }
                ]
              },
              {
                label: 'Auto Save',
                checkbox: true,
                checked: checked(),
                callback: () => {
                  setChecked(!checked());
                }
              }
            ]
          },
          {
            label: 'Edit',
            submenu: [{ label: 'Undo' }, { label: 'Redo' }, { label: 'Cut' }, { label: 'Copy' }]
          },
          {
            label: 'Object',
            submenu: [
              { label: 'Transform' },
              { label: 'Arrange' },
              { label: 'Group' },
              { label: 'Ungroup' }
            ]
          },
          {
            label: 'Type'
          }
        ]}
      />
    </div>
  );
};

export default TitleBar;
