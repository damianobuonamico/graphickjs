import { mat3, mat4, vec3 } from '@/math';
import { ShaderManager } from './shaderManager';
import { default as penShader } from './pen.glsl';

declare global {
  interface Window {
    OffscreenCanvas: OffscreenCanvas;
  }
}

function createOffscreenCanvas(size: vec2): HTMLCanvasElement {
  if ('OffscreenCanvas' in window) return new window.OffscreenCanvas(...size);
  return document.createElement('canvas');
}

class CanvasBackendFreehand {
  private m_canvas: HTMLCanvasElement;
  private m_gl: WebGL2RenderingContext;
  private m_shaders: ShaderManager;
  private m_arrayBuffer: WebGLBuffer;
  private m_indexBuffer: WebGLBuffer;
  private m_arrayBufferOffset: number;
  private m_indexBufferOffset: number;
  private m_indexBufferLength: number;
  private m_bufferSize = 2 ** 15;

  private m_dpr = 1;
  private m_zoom = 1;

  constructor() {
    this.m_canvas = createOffscreenCanvas([0, 0]);
    this.m_gl = this.m_canvas.getContext('webgl2', {
      antialias: true,
      alpha: true,
      premultipliedAlpha: true
    })!;

    this.m_gl.enable(this.m_gl.BLEND);
    this.m_gl.blendFunc(this.m_gl.SRC_ALPHA, this.m_gl.ONE_MINUS_SRC_ALPHA);

    this.m_shaders = new ShaderManager(this.m_gl);
    this.m_shaders.create('default', penShader);
  }

  get src(): CanvasImageSource {
    return this.m_canvas;
  }

  set size(value: vec2) {
    this.m_dpr = window.devicePixelRatio;

    this.m_canvas.width = value[0] * this.m_dpr;
    this.m_canvas.height = value[1] * this.m_dpr;

    this.m_gl.viewport(0, 0, value[0] * this.m_dpr, value[1] * this.m_dpr);
  }

  private flush() {
    this.endFrame();

    this.m_arrayBufferOffset = 0;
    this.m_arrayBuffer = this.m_gl.createBuffer()!;
    this.m_gl.bindBuffer(this.m_gl.ARRAY_BUFFER, this.m_arrayBuffer);
    this.m_gl.bufferData(this.m_gl.ARRAY_BUFFER, this.m_bufferSize, this.m_gl.DYNAMIC_DRAW);

    this.m_indexBufferOffset = 0;
    this.m_indexBuffer = this.m_gl.createBuffer()!;
    this.m_gl.bindBuffer(this.m_gl.ELEMENT_ARRAY_BUFFER, this.m_indexBuffer);
    this.m_gl.bufferData(this.m_gl.ELEMENT_ARRAY_BUFFER, this.m_bufferSize, this.m_gl.DYNAMIC_DRAW);

    this.m_indexBufferLength = 0;

    this.m_shaders.setAttribute('aPosition', 2, this.m_gl.FLOAT, 'default');
  }

  beginFrame(position: vec2, zoom: number): void {
    this.m_zoom = zoom;

    this.m_gl.clearColor(0, 0, 0, 0);
    this.m_gl.clear(this.m_gl.COLOR_BUFFER_BIT);

    const size = [this.m_canvas.width, this.m_canvas.height];

    const scaling = mat3.fromScaling([
      (zoom * this.m_dpr) / size[0],
      (-zoom * this.m_dpr) / size[1]
    ]);
    const translation = mat3.fromTranslation([
      -size[0] / zoom / this.m_dpr + 2 * position[0],
      -size[1] / zoom / this.m_dpr + 2 * position[1]
    ]);

    this.m_shaders.setGlobalUniform(
      'uViewProjectionMatrix',
      mat3.transpose(
        mat3.mul(mat3.mul(scaling, translation, scaling), mat3.fromScaling([2, 2])),
        scaling
      )
    );
    this.m_shaders.use('default');

    this.m_arrayBufferOffset = 0;
    this.m_arrayBuffer = this.m_gl.createBuffer()!;
    this.m_gl.bindBuffer(this.m_gl.ARRAY_BUFFER, this.m_arrayBuffer);
    this.m_gl.bufferData(this.m_gl.ARRAY_BUFFER, this.m_bufferSize, this.m_gl.STATIC_DRAW);

    this.m_indexBufferOffset = 0;
    this.m_indexBuffer = this.m_gl.createBuffer()!;
    this.m_gl.bindBuffer(this.m_gl.ELEMENT_ARRAY_BUFFER, this.m_indexBuffer);
    this.m_gl.bufferData(this.m_gl.ELEMENT_ARRAY_BUFFER, this.m_bufferSize, this.m_gl.STATIC_DRAW);

    this.m_indexBufferLength = 0;

    this.drawn = false;

    // this.m_arrayBufferArray = [];
    // this.m_indexBufferArray = [];

    this.m_shaders.setAttribute('aPosition', 2, this.m_gl.FLOAT, 'default');
  }

  private m_arrayBufferArray: number[];
  private m_indexBufferArray: number[];

  drawn = false;

  draw(positions: Float32Array, indices: number[]) {
    if (this.m_arrayBufferOffset + 2 * positions.byteLength > this.m_bufferSize) this.flush();

    // if (this.drawn) return;
    // this.m_arrayBufferArray.push(...positions);
    // this.m_indexBufferArray.push(...indices);

    this.m_gl.bufferSubData(
      this.m_gl.ARRAY_BUFFER,
      this.m_arrayBufferOffset,
      positions
      // Float32Array.from(positions.flat())
    );
    this.m_gl.bufferSubData(
      this.m_gl.ELEMENT_ARRAY_BUFFER,
      this.m_indexBufferOffset,
      Uint16Array.from(indices.map((index) => index + this.m_indexBufferLength))
      // Uint16Array.from(indices)
    );

    this.m_arrayBufferOffset += 2 * positions.byteLength;
    this.m_indexBufferOffset += 2 * indices.length;
    this.m_indexBufferLength += positions.length;

    this.drawn = true;
  }

  endFrame(): void {
    this.m_gl.drawElements(
      this.m_gl.TRIANGLES,
      this.m_indexBufferLength,
      this.m_gl.UNSIGNED_SHORT,
      0
    );

    this.m_gl.deleteBuffer(this.m_arrayBuffer);
    this.m_gl.deleteBuffer(this.m_indexBuffer);
  }
}

export default CanvasBackendFreehand;
