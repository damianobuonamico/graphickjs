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
      r="5.38938"
      fill={props.color || 'currentColor'}
      fill-opacity="0.35"
    />
    <path
      d="M7.49438 14.2109C11.2038 14.2109 14.2109 11.2038 14.2109 7.49438C14.2109 3.78494 11.2038 0.777832 7.49438 0.777832C3.78494 0.777832 0.777832 3.78494 0.777832 7.49438C0.777832 11.2038 3.78494 14.2109 7.49438 14.2109ZM7.5001 12.8417C10.4533 12.8417 12.8474 10.4476 12.8474 7.49437C12.8474 4.54113 10.4533 2.14706 7.5001 2.14706C4.54687 2.14706 2.1528 4.54113 2.1528 7.49437C2.1528 10.4476 4.54687 12.8417 7.5001 12.8417Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default CircleIcon;
