import { Component } from 'solid-js';
import { IconProps } from './types';

const JoinRoundIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M2.92306 2H2.9H2.5H2V2.5V8.5V9H2.5H6V12.5V13H6.5H12.5H13V12.5V12.1V12.0769C13 10.4163 13 9.13937 12.9166 8.1185C12.8323 7.08706 12.6604 6.27303 12.2915 5.54906C11.6684 4.32601 10.674 3.33163 9.45094 2.70846C8.72697 2.33958 7.91294 2.16768 6.8815 2.08341C5.86063 2 4.58372 2 2.92306 2ZM6.80007 3.08008C7.76966 3.1593 8.43745 3.31438 8.99695 3.59946C10.0318 4.12677 10.8732 4.96816 11.4005 6.00305C11.6856 6.56255 11.8407 7.23034 11.9199 8.19993C11.998 9.156 12 10.3626 12 12H7V8.5V8H6.5H3V3C4.63738 3.00005 5.844 3.00197 6.80007 3.08008Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default JoinRoundIcon;
