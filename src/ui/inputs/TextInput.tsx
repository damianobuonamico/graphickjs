import { Component } from 'solid-js';
import InputWrapper from './blocks/InputWrapper';
import TextBlock from './blocks/TextBlock';

const TextInput: Component<{
  onChange?(value: string | number): void;
  value: string | number;
  class?: string;
  leftIcon?: string;
  rightIcon?: string;
  type?: 'text' | 'hex';
}> = (props) => {
  return (
    <InputWrapper class={props.class}>
      <TextBlock
        onChange={props.onChange}
        value={props.value}
        leftIcon={props.leftIcon}
        rightIcon={props.rightIcon}
        type={props.type}
      />
    </InputWrapper>
  );
};

export default TextInput;
