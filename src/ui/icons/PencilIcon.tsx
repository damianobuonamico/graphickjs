import { Component } from 'solid-js';
import { IconProps } from './types';

const PencilIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M10.2071 2.55888L11.0588 1.70717L13.2929 3.94124L12.4412 4.7929L10.2071 2.55888Z"
      fill={props.color || 'currentColor'}
      stroke={props.color || 'currentColor'}
    />
    <path
      d="M1.63186 13.2446L2.60531 9.8375C2.61009 9.82078 2.61914 9.8056 2.63157 9.79345L8.42604 4.13152C8.46522 4.09323 8.52791 4.0936 8.56664 4.13233L10.8677 6.43336C10.9064 6.47209 10.9068 6.53478 10.8685 6.57396L5.20661 12.3685C5.19446 12.3809 5.17928 12.39 5.16256 12.3948L1.75548 13.3682C1.68005 13.3898 1.61031 13.32 1.63186 13.2446Z"
      stroke={props.color || 'currentColor'}
      stroke-width="1"
    />
    <path
      d="M3.06569 8.93431L8.43431 3.56569C8.74673 3.25327 9.25327 3.25327 9.56569 3.56569L11.4343 5.43431C11.7467 5.74673 11.7467 6.25327 11.4343 6.56569L6.06569 11.9343C5.75327 12.2467 5.24673 12.2467 4.93431 11.9343L3.06569 10.0657C2.75327 9.75327 2.75327 9.24673 3.06569 8.93431Z"
      fill={props.color || 'currentColor'}
    />
  </svg>
);

export default PencilIcon;
