import { Component } from 'solid-js';
import { IconProps } from './types';

const CapRoundIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M8.5 3.99997C10.433 3.99997 12 5.56697 12 7.49996V7.49998V7.5C12 9.43299 10.433 11 8.50001 11H8.49999H2.99991V7.99998H8.53757C8.81371 7.99998 9.03757 7.77613 9.03757 7.49998C9.03757 7.22384 8.81371 6.99998 8.53757 6.99998H2.99991V3.99998L8.49999 3.99997H8.5ZM13 7.49996C13 5.01468 10.9853 2.99997 8.5 2.99997H8.49998L2.49991 2.99998L1.99991 2.99998V3.49998V7.49998V11.5V12H2.49991H8.49999H8.50001C10.9853 12 13 9.98527 13 7.5V7.49998V7.49996Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default CapRoundIcon;
