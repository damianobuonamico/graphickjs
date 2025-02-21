import { classNames } from '@/utils/utils';
import { Component, Show } from 'solid-js';
import Button from './Button';

const Input: Component<{
  onChange?(id: string): void;
  class?: string;
  value: string;
  unit?: string;
  type?: string;
}> = (props) => {
  return (
    <>
      <input
        class={classNames('bg-transparent outline-none', props.class)}
        value={props.value}
        type={props.type ? props.type : 'text'}
      />
      <Show when={props.unit}>
        <a class="cursor-ew-resize">{props.unit}</a>
      </Show>
    </>
  );
};

export default Input;
