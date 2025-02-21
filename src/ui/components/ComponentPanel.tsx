import { classNames } from '@/utils/utils';
import { Component, JSX, Show } from 'solid-js';

const ComponentPanel: Component<{
  children?: JSX.Element;
  title: string;
  controls?: JSX.Element;
}> = (props) => {
  return (
    <div
      class={classNames(
        'bg-primary-800 h-fit w-full border-primary-600 border-b pb-2 pt-1.5 px-4 text-xs',
        {
          'pb-3': !props.children
        }
      )}
    >
      <div class="flex h-8 justify-between items-center">
        <a class="font-bold">{props.title}</a>
        {props.controls}
      </div>
      <Show when={props.children}>
        <div>{props.children}</div>
      </Show>
    </div>
  );
};

export default ComponentPanel;
