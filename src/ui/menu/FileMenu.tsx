import { createSignal, For, Component, onMount, createEffect } from 'solid-js';
import { onCleanup } from 'solid-js';
import Button from '@inputs/Button';
import { calculateAltLabel, MenuItem, MenuItems } from './ControlledMenu';
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

    if (alt()) {
      for (let i = 0; i < props.items.length; i++) {
        if (props.items[i].key === e.key) {
          if (props.items[i].submenu && props.items[i].submenu!.length) {
            setActive(i);
          } else {
            onClick(props.items[i]);
          }
        }
      }
    }

    return true;
  };

  const onKey = (e: KeyboardEvent) => {
    if (e.key === KEYS.ALT) {
      e.preventDefault();
      const last = alt();
      if (active() > -1) {
        setAlt(false);
        setActive(-1);
      } else {
        if (last === true || last === undefined) {
          setActive(-1);
          setFocus(false);
        } else setActive(0);
        setAlt(!last);
      }
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

  const onClose = () => {
    setFocus(false);
    setActive(-1);
    setAlt(false);
  };

  const onClick = (item: MenuItem) => {
    if (item.callback) item.callback();
    onClose();
  };

  return (
    <ul class="h-full flex flex-row items-center">
      <For each={props.items}>
        {(item, index) => (
          <li>
            {item.submenu && item.submenu.length ? (
              <Menu
                menuButton={{ label: item.label, variant: 'file-menu', key: item.key }}
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
                onClose={onClose}
                level={level + 1}
                keyCallback={keyCallback}
                alt={alt() === true}
              />
            ) : (
              <Button
                variant="file-menu"
                onHover={() => setActive(index())}
                onClick={() => onClick(item)}
              >
                {(alt() && item.key ? calculateAltLabel(item.label, item.key) : false) ||
                  item.label}
              </Button>
            )}
          </li>
        )}
      </For>
    </ul>
  );
};

export default FileMenu;
