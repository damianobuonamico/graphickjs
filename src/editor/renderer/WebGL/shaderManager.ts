import Shader from './shader';

export class ShaderManager {
  private _gl: WebGL2RenderingContext | WebGLRenderingContext;
  private _shaders: Map<string, Shader> = new Map();
  private _current: string | null = null;
  private _local: Set<string> = new Set();

  constructor(gl: WebGL2RenderingContext | WebGLRenderingContext) {
    this._gl = gl;
  }

  public create(id: string, source: string, global = true) {
    this._shaders.set(id, new Shader(this._gl, id, source));
    if (!global) this._local.add(id);
  }

  public use(id: string | null) {
    if (!id) {
      this._gl.useProgram(null);
      this._current = null;
      return;
    }

    const shader = this._shaders.get(id);
    if (!shader) return;

    shader.use();
    this._current = id;
  }

  public setGlobalUniform(uniform: string, value: VectorOrMatrix) {
    this._shaders.forEach((shader) => {
      if (!this._local.has(shader.id)) {
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
    const shaderId = id ?? this._current;
    if (!shaderId) return;

    const shader = this._shaders.get(shaderId);
    if (!shader) return;

    if (this._current !== shaderId) shader.use();
    shader.setAttribute(attribute, size, type);
  }

  public setUniform(uniform: string, value: VectorOrMatrix, id?: string) {
    const shaderId = id ?? this._current;

    if (!shaderId) return;

    const shader = this._shaders.get(shaderId);
    if (!shader) return;

    if (this._current !== shaderId) shader.use();
    shader.setUniform(uniform, value);
  }
}
