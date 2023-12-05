import { vec2 } from '@/math';
import { Component, createEffect, createSignal, JSX, onCleanup, onMount, Show } from 'solid-js';
import { Portal } from 'solid-js/web';

const Popover: Component<{ children: [JSX.Element, JSX.Element]; translate?: vec2 }> = (props) => {
  const [active, setActive] = createSignal(false);
  const [anchor, setAnchor] = createSignal([0, 0]);

  let toggleRef: HTMLButtonElement | undefined;
  let panelRef: HTMLDivElement | undefined;
  let mounted = false;

  createEffect(() => {
    if (active() && toggleRef) {
      const rect = toggleRef.getBoundingClientRect();
      const position: vec2 = [document.body.clientWidth - rect.x, rect.y];
      if (props.translate) vec2.add(position, props.translate, position);

      setAnchor(position);
    }
  });

  const onMouseDown = (e: MouseEvent) => {
    if (
      panelRef &&
      !panelRef.contains(e.target as Node) &&
      (!toggleRef || !toggleRef.contains(e.target as Node))
    )
      setActive(false);
  };

  onMount(() => {
    mounted = true;
    setTimeout(() => {
      if (mounted) {
        window.addEventListener('mousedown', onMouseDown);
      }
    }, 100);
  });

  onCleanup(() => {
    mounted = false;
    window.removeEventListener('mousedown', onMouseDown);
  });

  return (
    <>
      <button ref={toggleRef} onClick={() => setActive(!active())}>
        {props.children[0]}
      </button>
      <Show when={active()}>
        <Portal>
          <div
            ref={panelRef}
            class="absolute bg-primary-800 shadow-md rounded border-primary-600 border z-10"
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
