import {
  Component,
  createSignal,
  For,
  onCleanup,
  onMount,
  JSX
} from 'solid-js';
import { Button } from '@inputs';
import { CheckIcon } from '@icons';
import Menu from './Menu';
import { KEYS, getShortcutString } from '@utils/keys';
import MenuKeyCallback from './menuKeyCallback';

export interface MenuItem {
  label: string;
  key?: string;
  icon?: JSX.Element;
  callback?(): void;
  disabled?: boolean;
  submenu?: MenuItems;
  checked?: boolean;
  shortcut?: KeyBinding;
  active?: boolean;
}

export type MenuItems = MenuItem[];

export function calculateAltLabel(label: string, key: string) {
  var indices = [];
  for (var i = 0; i < label.length; i++) {
    if (label[i].toLowerCase() == key[0]) indices.push(i);
  }

  if (!indices.length) return label;

  return (
    <span>
      {label.substring(0, indices[key.length - 1])}
      <u>{label[indices[key.length - 1]]}</u>
      {label.substring(indices[key.length - 1] + 1, label.length)}
    </span>
  );
}

export function menuPrev(current: number, items: MenuItems): number {
  const prev = current < 1 ? items.length - 1 : current - 1;
  if (items[prev].disabled) return menuPrev(prev, items);
  return prev;
}

export function menuNext(current: number, items: MenuItems): number {
  const next = current >= items.length - 1 ? 0 : current + 1;
  if (items[next].disabled) return menuNext(next, items);
  return next;
}

const ControlledMenu: Component<{
  items: MenuItems;
  anchor: vec2;
  onClose(propagate: boolean): void;
  level?: number;
  keyCallback?: MenuKeyCallback;
  isSubMenu?: boolean;
  alt?: boolean;
  style?: JSX.CSSProperties;
  setActiveOnHover?: boolean;
}> = (props) => {
  const [active, setActive] = createSignal(props.isSubMenu ? 0 : -1);
  let menuRef: HTMLDivElement | undefined;
  let mounted = false;

  const onClick = (item: MenuItem) => {
    if (!item || item.disabled) return;
    if (item.callback) item.callback();
    props.onClose(true);
  };

  const onMouseDown = (e: MouseEvent) => {
    if (menuRef && !menuRef.contains(e.target as Node)) props.onClose(false);
  };

  const processKey = (e: KeyboardEvent) => {
    switch (e.key) {
      case KEYS.ARROW_DOWN: {
        setActive(menuNext(active(), props.items));
        return true;
      }
      case KEYS.ARROW_UP: {
        setActive(menuPrev(active(), props.items));
        return true;
      }
      case KEYS.ENTER: {
        onClick(props.items[active()]);
        return true;
      }
    }

    if (props.alt) {
      for (let i = 0; i < props.items.length; i++) {
        if (props.items[i].key === e.key) {
          if (props.items[i].submenu && props.items[i].submenu!.length) {
            if (!props.items[i].disabled) {
              setActive(i);
              setTimeout(
                () =>
                  window.dispatchEvent(
                    new KeyboardEvent('keydown', { key: KEYS.ARROW_RIGHT })
                  ),
                25
              );
            }
          } else onClick(props.items[i]);
          return true;
        }
      }
    }

    return false;
  };

  const onKey = (e: KeyboardEvent) => {
    if (props.keyCallback)
      props.keyCallback.register(() => processKey(e), props.level || 0);
  };

  onMount(() => {
    mounted = true;
    if (props.keyCallback) window.addEventListener('keydown', onKey);
    setTimeout(() => {
      if (mounted) {
        window.addEventListener('mousedown', onMouseDown);
      }
    }, 100);
  });

  onCleanup(() => {
    mounted = false;
    window.removeEventListener('mousedown', onMouseDown);
    if (props.keyCallback) window.removeEventListener('keydown', onKey);
  });

  return (
    <div
      class="fixed bg-primary-800 shadow-md rounded border-primary-600 border z-10"
      style={{ left: `${props.anchor[0]}px`, top: `${props.anchor[1]}px` }}
      ref={menuRef}
    >
      <ul class="py-2" style={props.style}>
        <For each={props.items}>
          {(item, index) => (
            <li class="flex">
              {item.label === 'separator' ? (
                <div
                  class="w-full h-[1px] my-1 bg-primary-600"
                  onMouseOver={
                    props.setActiveOnHover ? () => setActive(index()) : () => {}
                  }
                />
              ) : item.submenu && item.submenu.length ? (
                <Menu
                  menuButton={{
                    label: item.label,
                    variant: 'menu',
                    icon: item.icon,
                    key: item.key
                  }}
                  disabled={item.disabled}
                  items={item.submenu}
                  isSubMenu={true}
                  active={active() === index()}
                  onHover={
                    props.setActiveOnHover ? () => setActive(index()) : () => {}
                  }
                  onClose={(propagate: boolean) => {
                    setActive(-1);
                    if (propagate) {
                      props.onClose(propagate);
                    }
                  }}
                  level={props.level || 0 + 1}
                  keyCallback={props.keyCallback}
                  alt={props.alt || false}
                />
              ) : (
                <Button
                  variant={typeof item.label === 'string' ? 'menu' : 'tool'}
                  onClick={() => onClick(item)}
                  onHover={
                    props.setActiveOnHover ? () => setActive(index()) : () => {}
                  }
                  leftIcon={item.checked ? <CheckIcon /> : item.icon}
                  active={active() === index() || item.active}
                  disabled={item.disabled}
                >
                  {item.shortcut ? (
                    <div class="justify-between w-full flex">
                      {(props.alt && item.key && typeof item.label === 'string'
                        ? calculateAltLabel(item.label, item.key)
                        : false) || item.label}
                      <span class="text-primary-500 ml-6">
                        {getShortcutString(item.shortcut)}
                      </span>
                    </div>
                  ) : (
                    (props.alt && item.key && typeof item.label === 'string'
                      ? calculateAltLabel(item.label, item.key)
                      : false) || item.label
                  )}
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
