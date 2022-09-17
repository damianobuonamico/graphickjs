import { Component, createSignal } from 'solid-js';
import FileMenu from '@menu/FileMenu';
import { MenuItem } from '@menu/ControlledMenu';
import { KEYS } from '@utils/keys';
import actions from '@/editor/actions';
import getIcon from '../icons';

export function getModeComponent(mode: Mode, setMode: (mode: Mode) => void): MenuItem {
  switch (mode) {
    case 'photo':
      return {
        label: 'Photo',
        icon: 'affinityPhotoLogo',
        callback: () => {
          setMode(mode);
        },
        key: KEYS.H,
        shortcut: { key: KEYS.KEY_3, alt: true }
      };
    case 'publisher':
      return {
        label: 'Publisher',
        icon: 'affinityPublisherLogo',
        callback: () => {
          setMode(mode);
        },
        key: KEYS.P,
        shortcut: { key: KEYS.KEY_2, alt: true }
      };
    default:
      return {
        label: 'Designer',
        icon: 'affinityDesignerLogo',
        callback: () => {
          setMode(mode);
        },
        key: KEYS.D,
        shortcut: { key: KEYS.KEY_1, alt: true }
      };
  }
}

const TitleBar: Component<{ mode: Mode; setMode(mode: Mode): void; loading: boolean }> = (
  props
) => {
  const [checked, setChecked] = createSignal(true);

  return (
    <div class="bg-primary-800 h-8 w-full flex items-center border-primary-600 border-b">
      <FileMenu
        items={[
          {
            label: '',
            icon: props.loading
              ? getIcon('loading', { class: 'text-primary' })
              : getModeComponent(props.mode, props.setMode).icon,
            submenu: [
              getModeComponent('designer', props.setMode),
              getModeComponent('publisher', props.setMode),
              getModeComponent('photo', props.setMode)
            ]
          },
          {
            label: 'File',
            key: KEYS.F,
            submenu: [
              {
                label: 'New...',
                key: KEYS.N,
                shortcut: { key: KEYS.N, ctrl: true }
              },
              {
                label: 'New From Template...',
                key: KEYS.T,
                disabled: true,
                shortcut: { key: KEYS.N, ctrl: true, shift: true }
              },
              {
                label: 'separator',
                disabled: true
              },
              {
                label: 'Open...',
                key: KEYS.O,
                shortcut: { key: KEYS.O, ctrl: true }
              },
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
              {
                ...actions.undo,
                label: 'Undo',
                key: KEYS.U
              },
              {
                ...actions.redo,
                label: 'Redo',
                key: KEYS.R
              },
              {
                label: 'separator',
                disabled: true
              },
              {
                label: 'Cut',
                key: KEYS.T,
                shortcut: { key: KEYS.X, ctrl: true }
              },
              {
                label: 'Copy',
                key: KEYS.C,
                shortcut: { key: KEYS.C, ctrl: true }
              }
            ]
          },
          {
            label: 'Object',
            disabled: true,
            key: KEYS.O,
            submenu: [
              { label: 'Transform', key: KEYS.T },
              { label: 'Arrange', key: KEYS.A },
              {
                label: 'Group',
                key: KEYS.G,
                shortcut: { key: KEYS.G, ctrl: true }
              },
              {
                label: 'Ungroup',
                key: KEYS.U,
                shortcut: { key: KEYS.G, ctrl: true, shift: true }
              }
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
