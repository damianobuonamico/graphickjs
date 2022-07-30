import Shader from './shader';

export class ShaderManager {
  private m_gl: WebGL2RenderingContext | WebGLRenderingContext;
  private m_shaders: Map<string, Shader> = new Map();
  private m_current: string | null = null;
  private m_local: Set<string> = new Set();

  constructor(gl: WebGL2RenderingContext | WebGLRenderingContext) {
    this.m_gl = gl;
  }

  public create(id: string, source: string, global = true) {
    this.m_shaders.set(id, new Shader(this.m_gl, id, source));
    if (!global) this.m_local.add(id);
  }

  public use(id: string | null) {
    if (!id) {
      this.m_gl.useProgram(null);
      this.m_current = null;
      return;
    }

    const shader = this.m_shaders.get(id);
    if (!shader) return;

    shader.use();
    this.m_current = id;
  }

  public setGlobalUniform(uniform: string, value: VectorOrMatrix) {
    this.m_shaders.forEach((shader) => {
      if (!this.m_local.has(shader.id)) {
        shader.queueUniform(uniform, value);
      }
    });
  }

  public setAttribute(
    attribute: string,
    size: number,
    type: number,
    id?: string
  ) {
    const shaderId = id ?? this.m_current;
    if (!shaderId) return;

    const shader = this.m_shaders.get(shaderId);
    if (!shader) return;

    if (this.m_current !== shaderId) shader.use();
    shader.setAttribute(attribute, size, type);
  }

  public setUniform(uniform: string, value: VectorOrMatrix, id?: string) {
    const shaderId = id ?? this.m_current;

    if (!shaderId) return;

    const shader = this.m_shaders.get(shaderId);
    if (!shader) return;

    if (this.m_current !== shaderId) shader.use();
    shader.setUniform(uniform, value);
  }
}
