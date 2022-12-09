import { mat3, vec2 } from '@/math';
import { ShaderManager } from './shaderManager';
import { default as penShader } from './shaders/pen.glsl';
import { FrameBuffer } from './backbuffer';

declare global {
  interface Window {
    OffscreenCanvas: OffscreenCanvas;
  }
}

function createOffscreenCanvas(size: vec2): HTMLCanvasElement {
  if ('OffscreenCanvas' in window) return new window.OffscreenCanvas(...size);
  return document.createElement('canvas');
}

export enum Antialiasing {
  BROWSER = 0,
  MSAA = 1,
  FXAA = 2
}

function antialiasingToString(antialiasing: Antialiasing, samples: number) {
  switch (antialiasing) {
    case Antialiasing.BROWSER:
      return 'BROWSER';
    case Antialiasing.MSAA:
      return `MSAA ${samples}x`;
    case Antialiasing.FXAA:
      return 'FXAA';
  }
}

class CanvasBackendFreehand {
  private m_canvas: HTMLCanvasElement;
  private m_gl: WebGL2RenderingContext;
  private m_shaders: ShaderManager;

  private m_vertices: number;
  private m_samples: number;

  private m_frameBuffer: FrameBuffer;

  private m_vertexBuffer: WebGLBuffer;
  private m_indexBuffer: WebGLBuffer;

  private m_indexBufferLength: number;
  private m_vertexBufferLength: number;

  private m_maxVertexBufferSize: number = 500000;
  private m_maxVertexCount: number = this.m_maxVertexBufferSize / 8;
  private m_maxIndexCount: number = this.m_maxVertexCount * 3;
  private m_maxIndexBufferSize: number = this.m_maxIndexCount * 4;

  private m_indexBufferArray: Uint32Array;

  private m_antialasing: Antialiasing;
  private m_dpr = 1;

  constructor(antialiasing: Antialiasing) {
    this.m_canvas = createOffscreenCanvas([0, 0]);
    this.m_gl = this.m_canvas.getContext('webgl2', {
      antialias: antialiasing === Antialiasing.BROWSER,
      alpha: true,
      premultipliedAlpha: true
    })!;

    this.m_antialasing = antialiasing;

    this.m_gl.enable(this.m_gl.BLEND);
    this.m_gl.blendFunc(this.m_gl.SRC_ALPHA, this.m_gl.ONE_MINUS_SRC_ALPHA);

    this.m_shaders = new ShaderManager(this.m_gl);
    this.m_shaders.create('default', penShader);

    this.m_vertexBuffer = this.m_gl.createBuffer()!;
    this.m_gl.bindBuffer(this.m_gl.ARRAY_BUFFER, this.m_vertexBuffer);
    this.m_gl.bufferData(
      this.m_gl.ARRAY_BUFFER,
      this.m_maxVertexBufferSize,
      this.m_gl.DYNAMIC_DRAW
    );

    this.m_indexBuffer = this.m_gl.createBuffer()!;
    this.m_gl.bindBuffer(this.m_gl.ELEMENT_ARRAY_BUFFER, this.m_indexBuffer);
    this.m_gl.bufferData(
      this.m_gl.ELEMENT_ARRAY_BUFFER,
      this.m_maxIndexBufferSize,
      this.m_gl.DYNAMIC_DRAW
    );

    this.m_frameBuffer = new FrameBuffer(this.m_gl, this.m_shaders, [0, 0], antialiasing);

    this.m_samples =
      antialiasing === Antialiasing.MSAA
        ? this.m_gl.getParameter(this.m_gl.MAX_SAMPLES)
        : this.m_gl.getParameter(this.m_gl.SAMPLES);
  }

  get src(): CanvasImageSource {
    return this.m_canvas;
  }

  set size(value: vec2) {
    this.m_dpr = window.devicePixelRatio;

    this.m_canvas.width = value[0] * this.m_dpr;
    this.m_canvas.height = value[1] * this.m_dpr;

    this.m_gl.viewport(0, 0, value[0] * this.m_dpr, value[1] * this.m_dpr);

    this.m_frameBuffer.size = vec2.mulS(value, this.m_dpr);
  }

  private beginBatch() {
    this.m_indexBufferLength = 0;
    this.m_vertexBufferLength = 0;

    this.m_indexBufferArray = new Uint32Array(this.m_maxIndexCount);
  }

  private flush() {
    this.m_gl.bufferSubData(this.m_gl.ELEMENT_ARRAY_BUFFER, 0, this.m_indexBufferArray);

    this.m_gl.drawElements(
      this.m_gl.TRIANGLES,
      this.m_indexBufferLength,
      this.m_gl.UNSIGNED_INT,
      0
    );

    this.m_vertices += this.m_indexBufferLength;

    this.beginBatch();
  }

  beginFrame(position: vec2, zoom: number): void {
    if (this.m_antialasing === Antialiasing.BROWSER) {
      this.m_gl.clearColor(0, 0, 0, 0);
      this.m_gl.clear(this.m_gl.COLOR_BUFFER_BIT);
    } else {
      this.m_frameBuffer.bind();
    }

    this.m_vertices = 0;

    this.m_gl.bindBuffer(this.m_gl.ARRAY_BUFFER, this.m_vertexBuffer);

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
    this.m_shaders.setAttribute('aPosition', 2, this.m_gl.FLOAT, 'default');

    this.beginBatch();
  }

  endFrame(stats?: RendererStats): void {
    this.flush();

    if (this.m_antialasing !== Antialiasing.BROWSER) this.m_frameBuffer.render();

    if (stats) {
      stats.vertices = this.m_vertices;
      stats.antialiasing = antialiasingToString(this.m_antialasing, this.m_samples);
    }
  }

  draw(vertices: Float32Array, indices: number[]): void {
    if (
      this.m_vertexBufferLength + vertices.length > this.m_maxVertexCount ||
      this.m_indexBufferLength + indices.length > this.m_maxIndexCount
    ) {
      this.flush();
    }

    this.m_gl.bufferSubData(this.m_gl.ARRAY_BUFFER, this.m_vertexBufferLength * 8, vertices);

    for (let i = 0, n = indices.length; i < n; ++i) {
      this.m_indexBufferArray[this.m_indexBufferLength + i] =
        indices[i] + this.m_vertexBufferLength;
    }

    this.m_vertexBufferLength += vertices.length;
    this.m_indexBufferLength += indices.length;
  }
}

export default CanvasBackendFreehand;
