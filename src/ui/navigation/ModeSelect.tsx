import { Component, JSX } from 'solid-js';
import { Menu } from '../menu';
import { MenuItems } from '../menu/ControlledMenu';
import MenuKeyCallback from '../menu/menuKeyCallback';

export interface ModeSelectOption {
  icon: JSX.Element;
  id: string;
}

const ModeSelect: Component<{
  options: ModeSelectOption[];
  onChange(id: string): void;
  active: boolean;
  onHover(): void;
  onMouseDown(): void;
  onClose(propagate: boolean): void;
  level: number;
  keyCallback: MenuKeyCallback;
  alt: boolean;
  expanded?: boolean;
  disabled?: boolean;
}> = (props) => {
  const options: MenuItems = props.options.map((option) => {
    return { label: option.id };
  });

  return (
    <Menu
      menuButton={{ label: 'mode', variant: 'file-menu' }}
      items={options}
      active={props.active}
      onHover={props.onHover}
      onMouseDown={props.onMouseDown}
      onClose={props.onClose}
      level={props.level}
      keyCallback={props.keyCallback}
      alt={props.alt}
      expanded={props.expanded}
      disabled={props.disabled}
    />
  );
};

export default ModeSelect;
