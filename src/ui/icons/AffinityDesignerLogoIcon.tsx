import { Component } from 'solid-js';
import { IconProps } from './types';

const AffinityDesignerLogoIcon: Component<IconProps> = (props) => (
  <svg
    width="18"
    height="18"
    viewBox="0 0 48 48"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      fill="#0262b8"
      d="M42,38l-17.326,4h-5.739L6,38V9c0-1.105,0.895-2,2-2h32c1.105,0,2,0.895,2,2V38z"
    />
    <path
      fill="#103262"
      d="M26.08,29h-6.188c-1.391,0-2.256-1.51-1.553-2.71l3.201-5.46L26.08,27V29z M27.32,7V5l-6.34,1 L6,31.56V39c0,1.1,0.9,2,2,2h12.52l-6.67-11.01L27.32,7z M40,6H29.64l-6.93,10.82v2L28.37,29H42V8C42,6.9,41.1,6,40,6z M22.81,42 L40,41c1.1,0,2-0.9,2-2v-9l-25.3-1v2L22.81,42z M18.09,25.205h2.341v2.136H18.09V25.205z"
    />
    <linearGradient
      id="10Wni_zEFajFzVcdpZuOPa"
      x1="24"
      x2="24"
      y1="40"
      y2="5"
      gradientUnits="userSpaceOnUse"
    >
      <stop offset="0" stop-color="#33bff0" />
      <stop offset="1" stop-color="#54daff" />
    </linearGradient>
    <path
      fill="url(#10Wni_zEFajFzVcdpZuOPa)"
      d="M26.08,27h-6.188c-1.391,0-2.256-1.51-1.553-2.71l3.201-5.46L26.08,27z M27.32,5h-6.34 L6,30.56V40h14.52l-6.67-12.01L27.32,5z M40,5H29.64l-6.93,11.82L28.37,27H42V7C42,5.9,41.1,5,40,5z M22.81,40H42V29H16.7L22.81,40 z"
    />
    <path
      fill="#268fbf"
      d="M42,38v2c0,1.1-0.9,2-2,2H22.81v-2H40C41.105,40,42,39.105,42,38L42,38z"
    />
    <path
      fill="#268fbf"
      d="M6,38L6,38c0,1.105,0.895,2,2,2h12.52v2H8c-1.1,0-2-0.9-2-2V38z"
    />
  </svg>
);

export default AffinityDesignerLogoIcon;
