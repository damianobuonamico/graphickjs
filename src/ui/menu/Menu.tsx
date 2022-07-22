import { Component, createSignal, Show, createEffect, onMount, onCleanup } from 'solid-js';
import { vec2 } from '@math';
import { ChevronRightIcon } from '@icons';
import { Button } from '@inputs';
import { ButtonVariant } from '@inputs/Button';
import ControlledMenu, { MenuItems } from './ControlledMenu';
import { KEYS } from '@utils/keys';
import MenuKeyCallback from './menuKeyCallback';

const Menu: Component<{
  menuButton: { label: string; variant: ButtonVariant };
  items: MenuItems;
  isSubMenu?: boolean;
  active: boolean;
  onHover(): void;
  onMouseDown?(): void;
  onClose(propagate: boolean): void;
  level: number;
  keyCallback: MenuKeyCallback;
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

  return (
    <>
      <Button
        variant={props.menuButton.variant}
        onHover={onHover}
        onMouseDown={props.onMouseDown}
        ref={menuButtonRef}
        rightIcon={props.isSubMenu ? <ChevronRightIcon /> : undefined}
        active={props.active}
      >
        {props.menuButton.label}
      </Button>
      <Show when={props.active && !locked()}>
        <ControlledMenu
          items={props.items}
          anchor={anchor()}
          onClose={props.onClose}
          level={props.level + 1}
          keyCallback={props.keyCallback}
          isSubMenu={props.isSubMenu}
        />
      </Show>
    </>
  );
};

export default Menu;
