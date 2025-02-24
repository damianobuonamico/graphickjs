import { vec2 } from '@/math';
import { Component, createEffect, createSignal, JSX, onCleanup, onMount, Show } from 'solid-js';
import { Portal } from 'solid-js/web';

// Global stack to track active popovers
const activePopovers: HTMLElement[] = [];

const Popover: Component<{
  children: [JSX.Element, JSX.Element];
  translate?: vec2;
  stopPropagation?: boolean;
  positioning?: 'left' | 'over';
  closeFn?: { fn: () => void };
}> = (props) => {
  const positioning = props.positioning || 'left';

  const [active, setActive] = createSignal(false);
  const [anchor, setAnchor] = createSignal([0, 0]);

  let toggleRef: HTMLButtonElement | undefined;
  let panelRef: HTMLDivElement | undefined;

  const close = () => {
    setActive(false);
    activePopovers.pop();
  };

  if (props.closeFn) props.closeFn.fn = close;

  const onMouseDown = (e: MouseEvent) => {
    if (
      !panelRef ||
      !activePopovers.length ||
      activePopovers[activePopovers.length - 1] !== panelRef
    ) {
      return;
    }

    if (
      !panelRef.contains(e.target as Node) &&
      (!toggleRef || !toggleRef.contains(e.target as Node))
    ) {
      close();

      if (props.stopPropagation) e.stopPropagation();
    }
  };

  const onKeyDown = (e: KeyboardEvent) => {
    if (
      e.key === 'Escape' &&
      active() &&
      activePopovers.length &&
      activePopovers[activePopovers.length - 1] === panelRef
    ) {
      close();

      if (props.stopPropagation) e.stopPropagation();
    }
  };

  createEffect(() => {
    if (active()) {
      document.addEventListener('pointerdown', onMouseDown);
      document.addEventListener('keydown', onKeyDown);

      if (toggleRef) {
        const rect = toggleRef.getBoundingClientRect();

        let position: vec2;

        if (positioning === 'left') {
          position = [document.body.clientWidth - rect.x, rect.y];
        } else {
          position = [document.body.clientWidth - rect.x - rect.width, rect.y];
        }

        if (props.translate) vec2.add(position, props.translate, position);

        setAnchor(position);
      }

      if (!activePopovers.includes(panelRef!)) activePopovers.push(panelRef!);
    } else {
      document.removeEventListener('pointerdown', onMouseDown);
      document.removeEventListener('keydown', onKeyDown);
    }
  });

  onCleanup(() => {
    document.removeEventListener('pointerdown', onMouseDown);
    document.removeEventListener('keydown', onKeyDown);

    activePopovers.includes(panelRef!) && activePopovers.pop();
  });

  return (
    <>
      <button
        ref={toggleRef}
        onClick={() => {
          if (active()) {
            activePopovers.pop();
          }

          setActive((active) => !active);
        }}
      >
        {props.children[0]}
      </button>
      <Show when={active()}>
        <Portal>
          <div
            data-popover-active
            ref={panelRef}
            class="absolute bg-primary-800 shadow-md rounded border-primary-600 border z-[1000]"
            style={{
              top: anchor()[1] + 'px',
              right: anchor()[0] + 'px'
            }}
          >
            {props.children[1]}
          </div>
        </Portal>
      </Show>
    </>
  );
};

export default Popover;
