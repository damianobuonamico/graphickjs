import SceneManager from '@/editor/scene';
import { vec2, mat4, vec3 } from '@math';
import { createVertices } from '../geometry';
import BackBuffer from './backbuffer';
import { ShaderManager } from './shaderManager';
import { default as defaultshader } from './shaders/default.glsl';

class CanvasGL implements Canvas {
  private m_container: HTMLDivElement;
  private m_canvas: HTMLCanvasElement;
  private m_gl: WebGL2RenderingContext;
  private m_shaders: ShaderManager;
  private m_frameBuffer: BackBuffer;

  private m_options = {
    resolution: 1,
    antialiasing: 'ON'
  };
  private m_offset: vec2;

  constructor() {}

  get container() {
    return this.m_container;
  }

  set container(div: HTMLDivElement) {
    this.m_container = div;
    this.resize();
  }

  get DOM() {
    return this.m_canvas;
  }

  get offset(): vec2 {
    return vec2.clone(this.m_offset);
  }

  set offset(value: vec2) {
    this.m_offset = value;
    this.m_canvas.style.left = value[0] + 'px';
    this.m_canvas.style.top = value[1] + 'px';
  }

  get size(): vec2 {
    return [this.m_canvas.width, this.m_canvas.height];
  }

  set size(value: vec2) {
    this.m_canvas.width = value[0] * this.m_options.resolution;
    this.m_canvas.height = value[1] * this.m_options.resolution;

    this.m_canvas.style.width = value[0] + 'px';
    this.m_canvas.style.height = value[1] + 'px';

    this.m_gl.viewport(
      0,
      0,
      value[0] * this.m_options.resolution,
      value[1] * this.m_options.resolution
    );
    this.m_frameBuffer.size = vec2.mul(value, this.m_options.resolution);
  }

  public setup(canvas: HTMLCanvasElement) {
    this.m_canvas = canvas;
    this.m_gl = canvas.getContext('webgl2', {
      antialias:
        this.m_options.antialiasing === 'FXAA' || this.m_options.antialiasing === 'OFF'
          ? false
          : true,
      alpha: false,
      premultipliedAlpha: true
    })!;

    this.m_shaders = new ShaderManager(this.m_gl);
    this.m_shaders.create('default', defaultshader);

    this.m_frameBuffer = new BackBuffer(this.m_gl, this.m_shaders, this.size);

    this.m_gl.enable(this.m_gl.BLEND);
    this.m_gl.blendFunc(this.m_gl.SRC_ALPHA, this.m_gl.ONE_MINUS_SRC_ALPHA);
  }

  public resize() {
    this.offset = [this.m_container.offsetLeft, this.m_container.offsetTop];
    this.size = [this.m_container.offsetWidth, this.m_container.offsetHeight];
  }

  public clear({ color, depth }: { color?: vec4; depth?: boolean }) {
    const gl = this.m_gl;
    let clearFlag = 0;

    if (depth) {
      clearFlag = clearFlag | gl.DEPTH_BUFFER_BIT;
    }

    if (color) {
      gl.clearColor(color[0] || 0, color[1] || 0, color[2] || 0, color[3] || 0);
      clearFlag = clearFlag | gl.COLOR_BUFFER_BIT;
    }

    gl.clear(clearFlag);
  }

  private setBuffer(buffer?: BackBuffer) {
    const gl = this.m_gl;

    if (buffer) {
      gl.bindFramebuffer(gl.FRAMEBUFFER, buffer.frameBuffer);
      gl.clear(gl.COLOR_BUFFER_BIT);
    } else {
      gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    }
  }

  private geometry(
    positions: Float32Array,
    indices: Uint16Array,
    modelMatrix?: mat4,
    transformMatrix?: mat4
  ) {
    const gl = this.m_gl;

    this.m_shaders.use('default');

    if (modelMatrix) this.m_shaders.setUniform('uModelMatrix', modelMatrix, 'default');
    if (transformMatrix) this.m_shaders.setUniform('uTransformMatrix', transformMatrix, 'default');

    const buffer = gl.createBuffer()!;
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.bufferData(gl.ARRAY_BUFFER, positions, gl.STATIC_DRAW);

    this.m_shaders.setAttribute('aPosition', 2, gl.FLOAT, 'default');
    this.m_shaders.setUniform('uColor', [1.0, 1.0, 1.0, 1.0], 'default');

    const indexBuffer = gl.createBuffer()!;
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);

    gl.drawElements(gl.TRIANGLES, indices.length, gl.UNSIGNED_SHORT, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
  }

  public rect({
    pos,
    size,
    centered = false
  }: {
    pos: vec2;
    size: vec2 | number;
    centered?: boolean;
  }) {
    size = typeof size === 'number' ? [size, size] : size;
    if (centered) vec2.mul(size, 0.5, true);
    const vertices = createVertices(
      'rectangle',
      typeof size === 'number' ? [size, size] : size,
      false,
      centered
    );

    this.geometry(
      vec2.join(vertices),
      new Uint16Array([0, 1, 2, 2, 3, 0]),
      mat4.fromTranslation(vec3.fromValues(0, 0, 0)),
      mat4.fromTranslation(vec3.fromValues(pos[0], pos[1], 0))
    );
  }

  public beginFrame(): void {
    if (this.m_options.antialiasing === 'FXAA') this.setBuffer(this.m_frameBuffer);

    this.clear({ color: [0.0, 0.0, 0.0, 1.0] });

    this.m_shaders.setGlobalUniform(
      'uViewMatrix',
      mat4.translate(
        mat4.fromScaling(vec3.fromValues(1, 1, 1)),
        vec3.fromValues(SceneManager.viewport.position[0], SceneManager.viewport.position[1], 1)
      )
    );

    this.m_shaders.setGlobalUniform(
      'uProjectionMatrix',
      mat4.translate(
        mat4.fromScaling(
          vec3.fromValues(
            1 / (this.size[0] / 2 / this.m_options.resolution),
            -1 / (this.size[1] / 2 / this.m_options.resolution),
            1
          )
        ),
        vec3.fromValues(
          -this.size[0] / 2 / this.m_options.resolution,
          -this.size[1] / 2 / this.m_options.resolution,
          0
        )
      )
    );
  }

  public endFrame(): void {
    if (this.m_options.antialiasing === 'FXAA') {
      this.setBuffer();
      this.m_frameBuffer.render();
    }
  }
}

export default CanvasGL;
