interface FillComponentCollection {
  color: { value: ColorComponent; mixed: boolean; visible: boolean };
}

interface FillOptions {
  id?: string;
  style?: 'solid';
  color?: vec4 | string;
  visible?: boolean;
}

interface FillComponentObject {
  id?: string;
  style?: 'solid';
  color?: vec4;
  visible?: boolean;
}

interface FillComponent {
  id: string;
  style: 'solid';
  color: ColorComponent;
  visible: boolean;
  tempVisible: boolean;

  asObject(): FillComponentObject;
}
