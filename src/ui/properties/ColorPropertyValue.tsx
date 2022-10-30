import SelectionManager from '@/editor/selection';
import { nanoid } from 'nanoid';
import { Component, createEffect, createSignal } from 'solid-js';
import PropertyValue from './PropertyValue';

const ColorPropertyValue: Component<{ value: string; onChange: (color: string) => void }> = (
  props
) => {
  const id = nanoid();

  return (
    <PropertyValue rightPadding={false}>
      <div class="flex">
        <input
          type="color"
          class="hidden"
          id={id}
          value={props.value}
          // TODO: replace with onInput
          onChange={(e) => props.onChange((e.target as HTMLInputElement).value)}
        />
        <label
          class="w-[18px] h-[18px] bg-purple-300 cursor-pointer rounded-sm"
          for={id}
          style={{
            'background-color': props.value
          }}
        />
        <input
          class="ml-1.5 w-16 bg-transparent outline-none"
          type="text"
          value={props.value.replace('#', '').toUpperCase()}
        />
      </div>
      <input class="ml-1.5 w-10 bg-transparent outline-none" type="text" value="100%" />
    </PropertyValue>
  );
};

export default ColorPropertyValue;
