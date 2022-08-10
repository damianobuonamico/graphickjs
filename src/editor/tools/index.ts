import onPanPointerDown from './pan';
import onPenPointerDown from './pen';
import onPolygonPointerDown from './polygon';
import onSelectPointerDown from './select';
import onZoomPointerDown from './zoom';

const tools = {
  select: { callback: onSelectPointerDown },
  vselect: { callback: () => {} },
  pen: { callback: onPenPointerDown, data: {} },
  rectangle: { callback: () => onPolygonPointerDown('rectangle') },
  ellipse: { callback: () => onPolygonPointerDown('ellipse') },
  pan: { callback: onPanPointerDown },
  zoom: { callback: onZoomPointerDown }
};

export function getToolData(tool: Tool): ToolData {
  return (tools as any)[tool] || tools.select;
}
