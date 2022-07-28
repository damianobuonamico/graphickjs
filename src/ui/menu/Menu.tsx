import {
  Component,
  createSignal,
  Show,
  createEffect,
  onMount,
  onCleanup,
  JSX
} from 'solid-js';
import { vec2 } from '@math';
import { ChevronRightIcon } from '@icons';
import { Button } from '@inputs';
import { ButtonVariant } from '@inputs/Button';
import ControlledMenu, { calculateAltLabel, MenuItems } from './ControlledMenu';
import { KEYS } from '@utils/keys';
import MenuKeyCallback from './menuKeyCallback';

const Menu: Component<{
  menuButton: {
    label: string | JSX.Element;
    variant: ButtonVariant;
    icon?: JSX.Element;
    key?: string;
  };
  items: MenuItems;
  isSubMenu?: boolean;
  active: boolean;
  onHover(): void;
  onMouseDown?(): void;
  onClose(propagate: boolean): void;
  level: number;
  keyCallback?: MenuKeyCallback;
  alt: boolean;
  expanded?: boolean;
  disabled?: boolean;
}> = (props) => {
  const [anchor, setAnchor] = createSignal(vec2.create());
  const [locked, setLocked] = createSignal(props.level !== 1);
  let menuButtonRef: HTMLButtonElement | undefined;

  const processKey = (e: KeyboardEvent) => {
    if (props.level === 1) return false;
    if (!props.active) return false;
    switch (e.key) {
      case KEYS.ENTER:
      case KEYS.ARROW_RIGHT: {
        if (!locked()) return false;
        setLocked(false);
        return true;
      }
      case KEYS.ARROW_LEFT: {
        if (locked()) return false;
        setLocked(true);
        return true;
      }
    }
    setLocked(true);
    return false;
  };

  const onKey = (e: KeyboardEvent) => {
    if (props.keyCallback)
      props.keyCallback.register(() => processKey(e), props.level);
  };

  onMount(() => {
    window.addEventListener('keydown', onKey);
  });

  onCleanup(() => {
    window.removeEventListener('keydown', onKey);
  });

  createEffect(() => {
    if (props.active && menuButtonRef) {
      const rect = menuButtonRef.getBoundingClientRect();
      setAnchor(
        props.isSubMenu
          ? vec2.fromValues(rect.x + rect.width, rect.y - 9)
          : vec2.fromValues(rect.x, rect.y + rect.height - 1)
      );
    }
  });

  const onHover = () => {
    setLocked(false);
    props.onHover();
  };

  return (
    <>
      <Button
        variant={props.menuButton.variant}
        disabled={props.disabled}
        onHover={onHover}
        onMouseDown={() => {
          if (props.onMouseDown) props.onMouseDown();
          else setLocked(!locked());
        }}
        ref={menuButtonRef}
        leftIcon={props.menuButton.icon}
        rightIcon={props.isSubMenu ? <ChevronRightIcon /> : undefined}
        active={props.active}
        style={
          props.menuButton.variant === 'file-menu' &&
          props.menuButton.label === ''
            ? {
                width: '2rem',
                height: '2rem',
                padding: '0',
                'margin-left': '4px',
                'margin-right': '3px'
              }
            : undefined
        }
      >
        {(props.alt &&
        props.menuButton.key &&
        typeof props.menuButton.label === 'string'
          ? calculateAltLabel(props.menuButton.label, props.menuButton.key)
          : false) || props.menuButton.label}
      </Button>
      <Show
        when={
          props.active &&
          !locked() &&
          props.expanded !== false &&
          !props.disabled
        }
      >
        <ControlledMenu
          items={props.items}
          anchor={anchor()}
          onClose={props.onClose}
          level={props.level + 1}
          keyCallback={props.keyCallback}
          isSubMenu={props.isSubMenu}
          alt={props.alt}
          setActiveOnHover={true}
        />
      </Show>
    </>
  );
};

export default Menu;
