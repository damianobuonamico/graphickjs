import { Component } from 'solid-js';
import { IconProps } from './types';

const JoinMiterIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M13 2H12.5H2.5H2V2.5V8.5V9H2.5H6V12.5V13H6.5H12.5H13V12.5V2.5V2ZM12 3V12H7V8.5V8H6.5H3V3H12Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default JoinMiterIcon;
