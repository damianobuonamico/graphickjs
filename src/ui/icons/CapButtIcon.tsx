import { Component } from 'solid-js';
import { IconProps } from './types';

const CapButtIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M11 2.99997L10.5 2.99997L4.49991 2.99999L3.99991 2.99999V3.49999V7.49999V11.5V12H4.49991H10.5H11V11.5V7.49998V3.49997V2.99997ZM10 3.99997V6.99998L4.99991 6.99999V3.99998L10 3.99997ZM4.99991 7.99999L10 7.99998V11H4.99991V7.99999Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default CapButtIcon;
