import { Component, createEffect, createSignal, For, onCleanup, onMount } from 'solid-js';
import { Button } from '@inputs';
import { CheckIcon } from '@icons';
import Menu from './Menu';
import { KEYS } from '@utils/keys';
import MenuKeyCallback from './menuKeyCallback';

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
  level: number;
  keyCallback: MenuKeyCallback;
  isSubMenu?: boolean;
}> = (props) => {
  const [active, setActive] = createSignal(props.isSubMenu ? 0 : -1);
  let menuRef: HTMLDivElement | undefined;
  let mounted = false;

  const onClick = (item: MenuItem) => {
    if (item.callback) item.callback();
    props.onClose(true);
  };

  const onMouseDown = (e: MouseEvent) => {
    if (menuRef && !menuRef.contains(e.target as Node)) props.onClose(false);
  };

  const processKey = (e: KeyboardEvent) => {
    switch (e.key) {
      case KEYS.ARROW_DOWN: {
        setActive(active() === props.items.length - 1 ? 0 : active() + 1);
        return true;
      }
      case KEYS.ARROW_UP: {
        setActive(active() === 0 ? props.items.length - 1 : active() - 1);
        return true;
      }
      case KEYS.ENTER: {
        console.log(active());
        onClick(props.items[active()]);
        return true;
      }
    }
    return false;
  };

  const onKey = (e: KeyboardEvent) => {
    props.keyCallback.register(() => processKey(e), props.level);
  };

  onMount(() => {
    mounted = true;
    window.addEventListener('keydown', onKey);
    setTimeout(() => {
      if (mounted) {
        window.addEventListener('mousedown', onMouseDown);
      }
    }, 100);
  });

  onCleanup(() => {
    mounted = false;
    window.removeEventListener('mousedown', onMouseDown);
    window.removeEventListener('keydown', onKey);
  });

  return (
    <div
      class="fixed bg-primary-800 shadow-md rounded border-primary-600 border"
      style={{ left: `${props.anchor[0]}px`, top: `${props.anchor[1]}px` }}
      ref={menuRef}
    >
      <ul class="py-2">
        <For each={props.items}>
          {(item, index) => (
            <li class="flex">
              {item.submenu && item.submenu.length ? (
                <Menu
                  menuButton={{ label: item.label, variant: 'menu' }}
                  items={item.submenu}
                  isSubMenu={true}
                  active={active() === index()}
                  onHover={() => setActive(index())}
                  onClose={(propagate: boolean) => {
                    setActive(-1);
                    if (propagate) {
                      props.onClose(propagate);
                    }
                  }}
                  level={props.level + 1}
                  keyCallback={props.keyCallback}
                />
              ) : (
                <Button
                  variant="menu"
                  onClick={() => onClick(item)}
                  onHover={() => setActive(index())}
                  leftIcon={item.checked && <CheckIcon />}
                  active={active() === index()}
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
