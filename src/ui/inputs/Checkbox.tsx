import { Component, createEffect, createMemo, createSignal } from 'solid-js';
import Button from './Button';
import getIcon from '../icons';
import { classNames } from '@/utils/utils';

// TODO: default checkbox
const Checkbox: Component<{
  onChange?(value: boolean): void;
  checked: boolean | 'mixed';
  checkedIcon?: string;
  uncheckedIcon?: string;
  class?: string;
}> = (props) => {
  const [checked, setChecked] = createSignal(props.checked);

  const icon = createMemo(() =>
    getIcon(checked() ? props.checkedIcon ?? '' : props.uncheckedIcon ?? '')
  );

  createEffect(() => {
    setChecked(props.checked);
  });

  const toggle = () => {
    const prev = checked();

    if (props.onChange) props.onChange(!prev);
    setChecked(!prev);
  };

  return (
    <Button
      variant="small-button"
      class={classNames({ 'text-primary-300': checked() === 'mixed' }, props.class)}
      onClick={toggle}
    >
      {icon()}
    </Button>
  );
};

export default Checkbox;
