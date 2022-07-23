import { Component, createSignal } from 'solid-js';
import FileMenu from '@menu/FileMenu';
import { KEYS } from '@utils/keys';

const TitleBar: Component = () => {
  const [checked, setChecked] = createSignal(true);
  return (
    <div class="bg-primary-800 h-8 w-full flex items-center border-primary-600 border-b">
      <FileMenu
        items={[
          {
            label: 'File',
            key: KEYS.F,
            submenu: [
              { label: 'New...', key: KEYS.N },
              { label: 'New From Template...', key: KEYS.T },
              { label: 'Open...', key: KEYS.O },
              {
                label: 'Open Recent Files',
                key: KEYS.F,
                submenu: [
                  { label: 'foo.gk' },
                  { label: 'bar.gk' },
                  {
                    label: 'Older',
                    key: KEYS.O,
                    submenu: [{ label: 'old_foo.gk' }, { label: 'old_bar.gk' }]
                  }
                ]
              },
              {
                label: 'Auto Save',
                key: KEYS.A,
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
            key: KEYS.E,
            submenu: [
              { label: 'Undo', key: KEYS.U },
              { label: 'Redo', key: KEYS.R },
              { label: 'Cut', key: KEYS.T },
              { label: 'Copy', key: KEYS.C }
            ]
          },
          {
            label: 'Object',
            key: KEYS.O,
            submenu: [
              { label: 'Transform', key: KEYS.T },
              { label: 'Arrange', key: KEYS.A },
              { label: 'Group', key: KEYS.G },
              { label: 'Ungroup', key: KEYS.U }
            ]
          },
          {
            label: 'Type',
            key: KEYS.T
          }
        ]}
      />
    </div>
  );
};

export default TitleBar;
