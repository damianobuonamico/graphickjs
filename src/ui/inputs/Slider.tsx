import { clamp } from '@/math';
import { classNames } from '@/utils/utils';
import { Component, createEffect, createSignal, JSX, Show } from 'solid-js';

const Slider: Component<{
  style?: JSX.CSSProperties;
  overlay?: JSX.CSSProperties;
  class?: string;
  min?: number;
  max?: number;
  onChange?(value: number): void;
  onInput?(value: number): void;
  value: number;
}> = (props) => {
  const [value, setValue] = createSignal(0);
  const [anchor, setAnchor] = createSignal(0);

  let sliderRef: HTMLButtonElement | undefined;
  let cursorRef: HTMLDivElement | undefined;

  let min = props.min || 0;
  let max = props.max || 100;

  createEffect(() => {
    setValue(clamp(props.value, min, max));
  });

  createEffect(() => {
    if (sliderRef && cursorRef) {
      setAnchor(
        ((sliderRef.getBoundingClientRect().width -
          cursorRef.getBoundingClientRect().width / 2 -
          2) /
          (max - min)) *
          (value() - min)
      );
    }
  });

  const onMouseDown = (e: MouseEvent) => {
    document.addEventListener('mousemove', onMouseMove);
    document.addEventListener('mouseup', onMouseUp);

    if (sliderRef && cursorRef) {
      const rect = sliderRef.getBoundingClientRect();
      const cursorWidth = cursorRef.getBoundingClientRect().width;

      const value = clamp(
        min + ((e.x - rect.x - cursorWidth / 2) / (rect.width - cursorWidth)) * (max - min),
        min,
        max
      );

      setValue(value);
      if (props.onInput) props.onInput(value);
    }
  };

  const onMouseMove = (e: MouseEvent) => {
    if (sliderRef && cursorRef) {
      const rect = sliderRef.getBoundingClientRect();
      const cursorWidth = cursorRef.getBoundingClientRect().width;

      const value = clamp(
        min + ((e.x - rect.x - cursorWidth / 2) / (rect.width - cursorWidth)) * (max - min),
        min,
        max
      );

      setValue(value);
      if (props.onInput) props.onInput(value);
    }
  };

  const onMouseUp = () => {
    document.removeEventListener('mousemove', onMouseMove);
    if (props.onChange) props.onChange(value());
  };

  return (
    <button
      ref={sliderRef}
      class={classNames('w-full m-0 h-2 rounded', props.class)}
      style={{
        'box-shadow': props.overlay
          ? undefined
          : 'rgba(0, 0, 0, 0.1) 0px 0px 0px 1px inset, rgba(0, 0, 0, 0.15) 0px 0px 4px inset',
        ...props.style
      }}
      onMouseDown={onMouseDown}
    >
      <Show when={!!props.overlay}>
        <div
          class="float-left w-full h-full rounded"
          style={{
            'box-shadow':
              'rgba(0, 0, 0, 0.1) 0px 0px 0px 1px inset, rgba(0, 0, 0, 0.15) 0px 0px 4px inset',
            ...props.overlay
          }}
        />
      </Show>
      <div
        ref={cursorRef}
        class="w-3 h-3 relative -left-0.5 -top-0.5 border-white border-2 rounded-md"
        style={{ transform: `translateX(${anchor()}px)` }}
      />
    </button>
  );
};

export default Slider;
