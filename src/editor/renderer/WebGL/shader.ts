import { vectorMatrixCallback } from '@math';

class Shader {
  private _gl: WebGL2RenderingContext | WebGLRenderingContext;
  private _program: WebGLProgram;
  private _locations: Map<string, number | WebGLUniformLocation> = new Map();
  private _uniformQueue: Array<{ uniform: string; value: VectorOrMatrix }> = [];

  public readonly id: string;

  constructor(
    gl: WebGL2RenderingContext | WebGLRenderingContext,
    id: string,
    source: string
  ) {
    this._gl = gl;
    this.id = id;

    const shaderSource = this.parse(source);

    const vertexShader = this._createShader(
      gl.VERTEX_SHADER,
      shaderSource.vertex
    );
    const fragmentShader = this._createShader(
      gl.FRAGMENT_SHADER,
      shaderSource.fragment
    );

    if (!vertexShader || !fragmentShader) return;

    const program = this._createProgram(vertexShader, fragmentShader);

    if (!program) return;

    this._program = program;
  }

  private _createShader(type: number, source: string) {
    const gl = this._gl;
    const shader = gl.createShader(type);

    if (!shader) return null;

    gl.shaderSource(shader, source);
    gl.compileShader(shader);

    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
      console.warn(
        'An error occurred compiling the shader: ' + gl.getShaderInfoLog(shader)
      );
      gl.deleteShader(shader);
      return null;
    }

    return shader;
  }

  private parse(source: string) {
    const parts = source.split('#fragment\n');
    return {
      vertex: parts[0].split('#vertex\n')[1],
      fragment: parts[1]
    };
  }

  private _createProgram(
    vertexShader: WebGLShader,
    fragmentShader: WebGLShader
  ) {
    const gl = this._gl;
    const program = gl.createProgram();

    if (!program) return null;

    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);
    gl.linkProgram(program);

    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
      console.warn(
        'Failed to link the program: ' + gl.getProgramInfoLog(program)
      );
      gl.deleteProgram(program);
      gl.deleteShader(vertexShader);
      gl.deleteShader(fragmentShader);
      return null;
    }

    return program;
  }

  public use() {
    this._gl.useProgram(this._program);

    if (this._uniformQueue.length) {
      for (let i = 0; i < this._uniformQueue.length; i++) {
        this.setUniform(
          this._uniformQueue[i].uniform,
          this._uniformQueue[i].value
        );
      }
      this._uniformQueue.length = 0;
    }
  }

  public queueUniform(uniform: string, value: VectorOrMatrix) {
    this._uniformQueue.push({ uniform, value });
  }

  public setUniform(uniform: string, value: VectorOrMatrix) {
    const gl = this._gl;
    let location = this._locations.get(uniform) as WebGLUniformLocation;

    if (!location) {
      location = gl.getUniformLocation(this._program, uniform)!;
      this._locations.set(uniform, location);
    }

    vectorMatrixCallback({
      value,
      int: (value: number) => gl.uniform1i(location, value),
      float: (value: number) => gl.uniform1f(location, value),
      vec2: (value: vec2) => gl.uniform2fv(location, value),
      vec3: (value: vec3) => gl.uniform3fv(location, value),
      vec4: (value: vec4) => gl.uniform4fv(location, value),
      mat4: (value: mat4) => gl.uniformMatrix4fv(location, false, value)
    })(value);
  }

  public setAttribute(attribute: string, size: number, type: number) {
    const gl = this._gl;
    let location = this._locations.get(attribute) as number;

    if (!location) {
      location = gl.getAttribLocation(this._program, attribute);
      this._locations.set(attribute, location);
    }

    gl.vertexAttribPointer(location, size, type, false, 0, 0);
    gl.enableVertexAttribArray(location);
  }
}

export default Shader;
