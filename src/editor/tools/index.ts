import onPanPointerDown from './pan';

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
    default:
      return {
        callback: () => {
          return {};
        }
      };
  }
}
