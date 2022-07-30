import { vectorMatrixCallback } from '@math';

class Shader {
  private m_gl: WebGL2RenderingContext | WebGLRenderingContext;
  private m_program: WebGLProgram;
  private m_locations: Map<string, number | WebGLUniformLocation> = new Map();
  private m_uniformQueue: Array<{ uniform: string; value: VectorOrMatrix }> = [];

  public readonly id: string;

  constructor(
    gl: WebGL2RenderingContext | WebGLRenderingContext,
    id: string,
    source: string
  ) {
    this.m_gl = gl;
    this.id = id;

    const shaderSource = this.parse(source);

    const vertexShader = this.createShader(
      gl.VERTEX_SHADER,
      shaderSource.vertex
    );
    const fragmentShader = this.createShader(
      gl.FRAGMENT_SHADER,
      shaderSource.fragment
    );

    if (!vertexShader || !fragmentShader) return;

    const program = this.createProgram(vertexShader, fragmentShader);

    if (!program) return;

    this.m_program = program;
  }

  private createShader(type: number, source: string) {
    const gl = this.m_gl;
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

  private createProgram(
    vertexShader: WebGLShader,
    fragmentShader: WebGLShader
  ) {
    const gl = this.m_gl;
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
    this.m_gl.useProgram(this.m_program);

    if (this.m_uniformQueue.length) {
      for (let i = 0; i < this.m_uniformQueue.length; i++) {
        this.setUniform(
          this.m_uniformQueue[i].uniform,
          this.m_uniformQueue[i].value
        );
      }
      this.m_uniformQueue.length = 0;
    }
  }

  public queueUniform(uniform: string, value: VectorOrMatrix) {
    this.m_uniformQueue.push({ uniform, value });
  }

  public setUniform(uniform: string, value: VectorOrMatrix) {
    const gl = this.m_gl;
    let location = this.m_locations.get(uniform) as WebGLUniformLocation;

    if (!location) {
      location = gl.getUniformLocation(this.m_program, uniform)!;
      this.m_locations.set(uniform, location);
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
    const gl = this.m_gl;
    let location = this.m_locations.get(attribute) as number;

    if (!location) {
      location = gl.getAttribLocation(this.m_program, attribute);
      this.m_locations.set(attribute, location);
    }

    gl.vertexAttribPointer(location, size, type, false, 0, 0);
    gl.enableVertexAttribArray(location);
  }
}

export default Shader;
