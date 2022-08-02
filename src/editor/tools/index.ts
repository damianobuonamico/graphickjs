import onPanPointerDown from './pan';
import onPolygonPointerDown from './polygon';
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
    case 'ellipse':
      return {
        callback: () => onPolygonPointerDown(tool)
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
