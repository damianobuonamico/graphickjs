import { Component } from 'solid-js';
import { IconProps } from './types';

const PolygonIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M7.49991 2.01871L13.4815 6.23674L11.1967 13.0617H3.80308L1.51831 6.23674L7.49991 2.01871Z"
      fill={props.color || 'currentColor'}
      fill-opacity="0.35"
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
    <path
      d="M8.07875 0.830827C7.7322 0.584997 7.26815 0.584996 6.9216 0.830826L0.604357 5.31206C0.245457 5.56665 0.0951523 6.02593 0.234097 6.44345L2.63723 13.6647C2.7732 14.0733 3.15546 14.3489 3.58607 14.3489H11.4143C11.8449 14.3489 12.2271 14.0733 12.3631 13.6647L14.7663 6.44345C14.9052 6.02593 14.7549 5.56665 14.396 5.31206L8.07875 0.830827ZM13.4678 6.23971L7.49984 2.03131L1.53191 6.23971L3.81146 13.049H11.1882L13.4678 6.23971Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default PolygonIcon;
