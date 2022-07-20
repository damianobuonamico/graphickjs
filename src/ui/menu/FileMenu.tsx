import { createSignal, For } from 'solid-js';
import { Component } from 'solid-js';
import Button from '@inputs/Button';
import { MenuItems } from './ControlledMenu';
import Menu from './Menu';

const FileMenu: Component<{ items: MenuItems }> = (props) => {
  const [focus, setFocus] = createSignal(false);
  const [active, setActive] = createSignal<string | null>(null);

  return (
    <ul class="h-full flex flex-row items-center">
      <For each={props.items}>
        {(item) => (
          <li>
            {item.submenu && item.submenu.length ? (
              <Menu
                menuButton={{ label: item.label, variant: 'file-menu' }}
                items={item.submenu}
                active={active() === item.label}
                onHover={() => {
                  if (focus()) setActive(item.label);
                }}
                onMouseDown={() => {
                  setFocus(true);
                  setActive(item.label);
                }}
                onClose={() => {
                  setFocus(false);
                  setActive(null);
                }}
              />
            ) : (
              <Button variant="file-menu" onHover={() => setActive(null)}>
                {item.label}
              </Button>
            )}
          </li>
        )}
      </For>
    </ul>
  );
};

export default FileMenu;
