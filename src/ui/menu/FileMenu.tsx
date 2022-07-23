import { createSignal, For, Component, onMount, createEffect } from 'solid-js';
import { onCleanup } from 'solid-js';
import Button from '@inputs/Button';
import { calculateAltLabel, MenuItem, MenuItems, menuNext, menuPrev } from './ControlledMenu';
import Menu from './Menu';
import { KEYS } from '@utils/keys';
import MenuKeyCallback from './menuKeyCallback';

const FileMenu: Component<{ items: MenuItems }> = (props) => {
  const [focus, setFocus] = createSignal(false);
  const [active, setActive] = createSignal(-1);
  const [alt, setAlt] = createSignal<boolean | undefined>(false);
  const [expanded, setExpanded] = createSignal(true);
  const level = 0;
  const keyCallback = new MenuKeyCallback();
  let menuRef: HTMLUListElement | undefined;

  const processKey = (e: KeyboardEvent) => {
    if (active() < 0) return false;
    switch (e.key) {
      case KEYS.ARROW_RIGHT: {
        setActive(menuNext(active(), props.items));
        break;
      }
      case KEYS.ARROW_LEFT: {
        setActive(menuPrev(active(), props.items));
        break;
      }
      case KEYS.ENTER: {
        if (!props.items[active()].submenu || !props.items[active()].submenu!.length) {
          onClick(props.items[active()]);
          break;
        }
      }
      case KEYS.ARROW_DOWN: {
        if (focus()) setExpanded(true);
        break;
      }
      case KEYS.ESCAPE: {
        onClose();
        break;
      }
    }

    if (alt()) {
      for (let i = 0; i < props.items.length; i++) {
        if (props.items[i].key === e.key) {
          if (props.items[i].submenu && props.items[i].submenu!.length) {
            setActive(i);
            setExpanded(true);
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
        setExpanded(true);
        onClose();
      } else {
        if (last === true || last === undefined) {
          setActive(-1);
          setFocus(false);
        } else {
          setActive(0);
          setFocus(true);
        }
        setAlt(!last);
        setExpanded(false);
      }
      return;
    }
    keyCallback.register(() => processKey(e), level);
  };

  const onMouseDown = (e: MouseEvent) => {
    if (menuRef && !menuRef.contains(e.target as Node) && !expanded()) onClose();
  };

  createEffect(() => {
    if (alt() === true) {
      window.addEventListener('click', onMouseDown);
    } else {
      window.removeEventListener('click', onMouseDown);
    }
  });

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
    if (item.disabled) return;
    if (item.callback) item.callback();
    onClose();
  };

  return (
    <ul ref={menuRef} class="h-full flex flex-row items-center">
      <For each={props.items}>
        {(item, index) => (
          <li>
            {item.submenu && item.submenu.length ? (
              <Menu
                menuButton={{ label: item.label, variant: 'file-menu', key: item.key }}
                disabled={item.disabled}
                items={item.submenu}
                active={active() === index()}
                onHover={() => {
                  if (focus()) {
                    setActive(index());
                    setExpanded(true);
                  }
                }}
                onMouseDown={() => {
                  if (item.disabled) return;
                  setFocus(true);
                  setActive(index());
                  setAlt(undefined);
                }}
                onClose={onClose}
                level={level + 1}
                keyCallback={keyCallback}
                alt={alt() === true}
                expanded={expanded()}
              />
            ) : (
              <Button
                variant="file-menu"
                disabled={item.disabled}
                onHover={() => setActive(index())}
                onClick={() => onClick(item)}
                active={focus() && active() === index()}
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
