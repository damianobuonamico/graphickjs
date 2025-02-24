import { Component } from 'solid-js';
import { IconProps } from './types';

const JoinBevelIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M7.70711 2H7.5H2.5H2V2.5V8.5V9H2.5H6V12.5V13H6.5H12.5H13V12.5V7.5V7.29289L12.8536 7.14645L7.85355 2.14645L7.70711 2ZM7.29289 3L12 7.70711V12H7V8.5V8H6.5H3V3H7.29289Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default JoinBevelIcon;
