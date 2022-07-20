import { Component, createSignal, Show, createEffect } from 'solid-js';
import { vec2 } from '@math';
import { ChevronRightIcon } from '@icons';
import { Button } from '@inputs';
import { ButtonVariant } from '@inputs/Button';
import ControlledMenu, { MenuItems } from './ControlledMenu';

const Menu: Component<{
  menuButton: { label: string; variant: ButtonVariant };
  items: MenuItems;
  isSubMenu?: boolean;
  active: boolean;
  onHover(): void;
  onMouseDown?(): void;
  onClose(propagate: boolean): void;
}> = (props) => {
  const [anchor, setAnchor] = createSignal(vec2.create());

  let menuButtonRef: HTMLButtonElement | undefined;

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

  return (
    <>
      <Button
        variant={props.menuButton.variant}
        onHover={props.onHover}
        onMouseDown={props.onMouseDown}
        ref={menuButtonRef}
        rightIcon={props.isSubMenu ? <ChevronRightIcon /> : undefined}
        active={props.active}
      >
        {props.menuButton.label}
      </Button>
      <Show when={props.active}>
        <ControlledMenu items={props.items} anchor={anchor()} onClose={props.onClose} />
      </Show>
    </>
  );
};

export default Menu;
