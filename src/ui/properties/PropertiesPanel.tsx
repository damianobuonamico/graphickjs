import { Component, createSignal } from 'solid-js';
import FillPropertyPanel from './FillPropertyPanel';
import StrokePropertyPanel from './StrokePropertyPanel';

const PropertiesPanel: Component<{}> = (props) => {
  return (
    <div class="bg-primary-800 h-full w-full border-primary-600 border-l">
      <FillPropertyPanel />
      <StrokePropertyPanel />
    </div>
  );
};

export default PropertiesPanel;
