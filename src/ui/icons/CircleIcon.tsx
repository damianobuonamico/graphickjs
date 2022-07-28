import { Component } from 'solid-js';
import { IconProps } from './types';

const CircleIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <circle
      cx="7.5"
      cy="7.5"
      r="5.5"
      fill={props.color || 'currentColor'}
      fill-opacity="0.35"
    />
    <path
      d="M7.5 14.25C11.2279 14.25 14.25 11.2279 14.25 7.5C14.25 3.77208 11.2279 0.75 7.5 0.75C3.77208 0.75 0.75 3.77208 0.75 7.5C0.75 11.2279 3.77208 14.25 7.5 14.25ZM7.5 12.8036C10.4291 12.8036 12.8036 10.4291 12.8036 7.49998C12.8036 4.5709 10.4291 2.19641 7.5 2.19641C4.57092 2.19641 2.19643 4.5709 2.19643 7.49998C2.19643 10.4291 4.57092 12.8036 7.5 12.8036Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default CircleIcon;
