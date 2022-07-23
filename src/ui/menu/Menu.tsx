import { Component, createSignal, Show, createEffect, onMount, onCleanup, JSX } from 'solid-js';
import { vec2 } from '@math';
import { ChevronRightIcon } from '@icons';
import { Button } from '@inputs';
import { ButtonVariant } from '@inputs/Button';
import ControlledMenu, { calculateAltLabel, MenuItems } from './ControlledMenu';
import { KEYS } from '@utils/keys';
import MenuKeyCallback from './menuKeyCallback';

const Menu: Component<{
  menuButton: { label: string; variant: ButtonVariant; key?: string };
  items: MenuItems;
  isSubMenu?: boolean;
  active: boolean;
  onHover(): void;
  onMouseDown?(): void;
  onClose(propagate: boolean): void;
  level: number;
  keyCallback: MenuKeyCallback;
  alt: boolean;
  expanded?: boolean;
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

  const altLabel: JSX.Element | false = props.menuButton.key
    ? calculateAltLabel(props.menuButton.label, props.menuButton.key)
    : false;

  return (
    <>
      <Button
        variant={props.menuButton.variant}
        onHover={onHover}
        onMouseDown={() => {
          if (props.onMouseDown) props.onMouseDown();
          else setLocked(!locked());
        }}
        ref={menuButtonRef}
        rightIcon={props.isSubMenu ? <ChevronRightIcon /> : undefined}
        active={props.active}
      >
        {(props.alt ? altLabel : false) || props.menuButton.label}
      </Button>
      <Show when={props.active && !locked() && props.expanded !== false}>
        <ControlledMenu
          items={props.items}
          anchor={anchor()}
          onClose={props.onClose}
          level={props.level + 1}
          keyCallback={props.keyCallback}
          isSubMenu={props.isSubMenu}
          alt={props.alt}
        />
      </Show>
    </>
  );
};

export default Menu;
