import onPanPointerDown from './pan';
import onZoomPointerDown from './zoom';

export function getToolData(tool: Tool): ToolData {
  switch (tool) {
    case 'vselect':
      return {
        callback: () => {
          return {};
        }
      };
    case 'pen':
      return {
        callback: () => {
          return {};
        }
      };
    case 'rectangle':
      return {
        callback: () => {
          return {};
        }
      };
    case 'ellipse':
      return {
        callback: () => {
          return {};
        }
      };
    case 'pan':
      return {
        callback: onPanPointerDown
      };
    case 'zoom':
      return {
        callback: onZoomPointerDown
      };
    default:
      return {
        callback: () => {
          return {};
        }
      };
  }
}
