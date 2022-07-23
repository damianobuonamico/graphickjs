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
              { label: 'New...', key: KEYS.N, shortcut: { key: KEYS.N, ctrl: true } },
              {
                label: 'New From Template...',
                key: KEYS.T,
                disabled: true,
                shortcut: { key: KEYS.N, ctrl: true, shift: true }
              },
              { label: 'Open...', key: KEYS.O, shortcut: { key: KEYS.O, ctrl: true } },
              {
                label: 'Open Recent Files',
                key: KEYS.F,
                submenu: [
                  { label: 'foo.gk', disabled: true },
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
                disabled: true,
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
              { label: 'Undo', key: KEYS.U, shortcut: { key: KEYS.Z, ctrl: true } },
              { label: 'Redo', key: KEYS.R, shortcut: { key: KEYS.Z, ctrl: true, shift: true } },
              { label: 'Cut', key: KEYS.T, shortcut: { key: KEYS.X, ctrl: true } },
              { label: 'Copy', key: KEYS.C, shortcut: { key: KEYS.C, ctrl: true } }
            ]
          },
          {
            label: 'Object',
            disabled: true,
            key: KEYS.O,
            submenu: [
              { label: 'Transform', key: KEYS.T },
              { label: 'Arrange', key: KEYS.A },
              { label: 'Group', key: KEYS.G, shortcut: { key: KEYS.G, ctrl: true } },
              { label: 'Ungroup', key: KEYS.U, shortcut: { key: KEYS.G, ctrl: true, shift: true } }
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
