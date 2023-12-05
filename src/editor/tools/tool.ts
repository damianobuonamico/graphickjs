import API from "@/wasm/loader";

declare global {
  interface Window {
    _set_tool(type: number): void;
  }
}

enum ToolType {
  Pan = 0,
  Zoom,
  Select,
  DirectSelect,
  Pen,
  Pencil,
  None,
}

class ToolState {
  private m_current: Tool;
  private m_active: Tool;

  private m_setTool: (tool: Tool) => void;

  constructor(setTool: (tool: Tool) => void, tool: Tool) {
    this.m_setTool = setTool;
    this.current = tool;

    window._set_tool = (type: ToolType) => {
      switch (type) {
        case ToolType.Pan:
          this.m_setTool("pan");
          break;
        case ToolType.Zoom:
          this.m_setTool("zoom");
          break;
        case ToolType.DirectSelect:
          this.m_setTool("directSelect");
          break;
        case ToolType.Pen:
          this.m_setTool("pen");
          break;
        case ToolType.Pencil:
          this.m_setTool("pencil");
          break;
        case ToolType.Select:
        default:
          this.m_setTool("select");
          break;
      }
    };
  }

  public get current() {
    return this.m_current;
  }

  public set current(tool: Tool) {
    switch (tool) {
      case "pan":
        API._set_tool(ToolType.Pan);
        break;
      case "zoom":
        API._set_tool(ToolType.Zoom);
        break;
      case "directSelect":
        API._set_tool(ToolType.DirectSelect);
        break;
      case "pen":
        API._set_tool(ToolType.Pen);
        break;
      case "pencil":
        API._set_tool(ToolType.Pencil);
        break;
      case "select":
      default:
        API._set_tool(ToolType.Select);
        break;
    }
  }

  public get active() {
    return this.m_active;
  }
}

export default ToolState;
