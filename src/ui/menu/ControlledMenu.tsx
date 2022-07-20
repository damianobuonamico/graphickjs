import { Component, createSignal, For, onCleanup, onMount } from 'solid-js';
import { Button } from '@inputs';
import Menu from './Menu';
import { CheckIcon } from '@icons';

export interface MenuItem {
  label: string;
  icon?: Component;
  callback?(): void;
  submenu?: MenuItems;
  checked?: boolean;
  checkbox?: boolean;
}

export type MenuItems = MenuItem[];

const ControlledMenu: Component<{
  items: MenuItems;
  anchor: vec2;
  onClose(propagate: boolean): void;
}> = (props) => {
  const [active, setActive] = createSignal<string | null>(null);
  let menuRef: HTMLDivElement | undefined;
  let mounted = false;

  const onClick = (item: MenuItem) => {
    if (item.callback) item.callback();
    if (!item.checkbox) props.onClose(true);
  };

  const onMouseDown = (e: MouseEvent) => {
    if (menuRef && !menuRef.contains(e.target as Node)) props.onClose(false);
  };

  onMount(() => {
    mounted = true;
    setTimeout(() => {
      if (mounted) {
        window.addEventListener('mousedown', onMouseDown);
      }
    }, 100);
  });

  onCleanup(() => {
    mounted = false;
    window.removeEventListener('mousedown', onMouseDown);
  });

  return (
    <div
      class="fixed bg-primary-800 shadow-md rounded border-primary-600 border"
      style={{ left: `${props.anchor[0]}px`, top: `${props.anchor[1]}px` }}
      ref={menuRef}
    >
      <ul class="py-2">
        <For each={props.items}>
          {(item) => (
            <li class="flex">
              {item.submenu && item.submenu.length ? (
                <Menu
                  menuButton={{ label: item.label, variant: 'menu' }}
                  items={item.submenu}
                  isSubMenu={true}
                  active={active() === item.label}
                  onHover={() => setActive(item.label)}
                  onClose={(propagate: boolean) => {
                    setActive(null);
                    if (propagate) {
                      props.onClose(propagate);
                    }
                  }}
                />
              ) : (
                <Button
                  variant="menu"
                  onClick={() => onClick(item)}
                  onHover={() => setActive(item.label)}
                  leftIcon={item.checked && <CheckIcon />}
                >
                  {item.label}
                </Button>
              )}
            </li>
          )}
        </For>
      </ul>
    </div>
  );
};

export default ControlledMenu;
