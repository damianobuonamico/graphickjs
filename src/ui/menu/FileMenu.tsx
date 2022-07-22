import { createSignal, For, Component, onMount, createEffect } from 'solid-js';
import { onCleanup } from 'solid-js';
import Button from '@inputs/Button';
import { MenuItems } from './ControlledMenu';
import Menu from './Menu';
import { KEYS } from '@utils/keys';
import MenuKeyCallback from './menuKeyCallback';

const FileMenu: Component<{ items: MenuItems }> = (props) => {
  const [focus, setFocus] = createSignal(false);
  const [active, setActive] = createSignal(-1);
  const [alt, setAlt] = createSignal<boolean | undefined>(false);
  const level = 0;
  const keyCallback = new MenuKeyCallback();

  const processKey = (e: KeyboardEvent) => {
    if (active() < 0) return false;
    switch (e.key) {
      case KEYS.ARROW_RIGHT: {
        setActive(active() === props.items.length - 1 ? 0 : active() + 1);
        break;
      }
      case KEYS.ARROW_LEFT: {
        setActive(active() === 0 ? props.items.length - 1 : active() - 1);
        break;
      }
      case KEYS.ESCAPE: {
        setActive(-1);
        break;
      }
    }
    return true;
  };

  const onKey = (e: KeyboardEvent) => {
    if (e.key === KEYS.ALT) {
      e.preventDefault();
      const last = alt();
      if (last === true || last === undefined) {
        setActive(-1);
        setFocus(false);
      } else setActive(0);
      setAlt(!last);
      return;
    }
    keyCallback.register(() => processKey(e), level);
  };

  onMount(() => {
    window.addEventListener('keydown', onKey);
  });

  onCleanup(() => {
    window.removeEventListener('keydown', onKey);
  });

  return (
    <ul class="h-full flex flex-row items-center">
      <For each={props.items}>
        {(item, index) => (
          <li>
            {item.submenu && item.submenu.length ? (
              <Menu
                menuButton={{ label: item.label, variant: 'file-menu' }}
                items={item.submenu}
                active={active() === index()}
                onHover={() => {
                  if (focus()) setActive(index());
                }}
                onMouseDown={() => {
                  setFocus(true);
                  setActive(index());
                  setAlt(undefined);
                }}
                onClose={() => {
                  setFocus(false);
                  setActive(-1);
                  setAlt(false);
                }}
                level={level + 1}
                keyCallback={keyCallback}
              />
            ) : (
              <Button variant="file-menu" onHover={() => setActive(index())}>
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
