import { classNames } from '@/utils/utils';
import { Component, JSX, Show } from 'solid-js';

const PropertyPanel: Component<{ children?: JSX.Element; title: string }> = (props) => {
  return (
    <div
      class={classNames(
        'bg-primary-800 h-fit w-full border-primary-600 border-b p-2 pt-3 text-xs',
        {
          'pb-3': !props.children
        }
      )}
    >
      <a class="font-semibold select-none ml-[7px]">{props.title}</a>
      <Show when={props.children}>
        <div class="mt-2">{props.children}</div>
      </Show>
    </div>
  );
};

export default PropertyPanel;
