import { Component } from 'solid-js';
import { IconProps } from './types';

const PointerIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M3 13.1099V1.89013C3 1.18332 3.8495 0.823768 4.35692 1.31581L12.5827 9.29236C13.0987 9.79266 12.7445 10.6667 12.0258 10.6667H7.79293C7.58502 10.6667 7.38528 10.7476 7.23602 10.8924L4.35692 13.6842C3.8495 14.1763 3 13.8167 3 13.1099Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default PointerIcon;
