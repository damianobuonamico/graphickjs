import { Antialiasing } from './backendWebGL';
import { ShaderManager } from './ShaderManager';
import { default as fxaa } from './shaders/fxaa.glsl';
import { default as msaa } from './shaders/msaa.glsl';

const FRAMEBUFFER = { RENDERBUFFER: 0, COLORBUFFER: 1 };

export class FrameBuffer {
  private m_gl: WebGL2RenderingContext;
  private m_shaders: ShaderManager;

  private m_texture: WebGLTexture;
  private m_frameBuffers: [WebGLFramebuffer, WebGLFramebuffer];
  private m_colorRenderbuffer: WebGLRenderbuffer;
  private m_vertexBuffer: WebGLBuffer;

  private m_antialiasing: Antialiasing;
  private m_samples: number;
  private m_size: vec2;

  constructor(
    gl: WebGL2RenderingContext,
    shaders: ShaderManager,
    size: vec2,
    antialiasing: Antialiasing
  ) {
    this.m_gl = gl;
    this.m_antialiasing = antialiasing;
    this.m_samples = antialiasing === Antialiasing.MSAA ? gl.getParameter(gl.MAX_SAMPLES) : 1;

    shaders.create('frame', antialiasing === Antialiasing.FXAA ? fxaa : msaa);

    const texture = gl.createTexture()!;
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, size[0], size[1], 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    gl.bindTexture(gl.TEXTURE_2D, null);

    const frameBuffers: [WebGLFramebuffer, WebGLFramebuffer] = [
      gl.createFramebuffer()!,
      gl.createFramebuffer()!
    ];

    const colorRenderbuffer = gl.createRenderbuffer()!;
    gl.bindRenderbuffer(gl.RENDERBUFFER, colorRenderbuffer);
    gl.renderbufferStorageMultisample(gl.RENDERBUFFER, this.m_samples, gl.RGBA8, size[0], size[1]);

    gl.bindFramebuffer(gl.FRAMEBUFFER, frameBuffers[FRAMEBUFFER.RENDERBUFFER]);
    gl.framebufferRenderbuffer(
      gl.FRAMEBUFFER,
      gl.COLOR_ATTACHMENT0,
      gl.RENDERBUFFER,
      colorRenderbuffer
    );
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    gl.bindFramebuffer(gl.FRAMEBUFFER, frameBuffers[FRAMEBUFFER.COLORBUFFER]);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    this.m_vertexBuffer = gl.createBuffer()!;
    gl.bindBuffer(gl.ARRAY_BUFFER, this.m_vertexBuffer);
    gl.bufferData(
      gl.ARRAY_BUFFER,
      new Float32Array([-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0]),
      gl.STATIC_DRAW
    );

    this.m_frameBuffers = frameBuffers;
    this.m_colorRenderbuffer = colorRenderbuffer;
    this.m_texture = texture;
    this.m_shaders = shaders;
  }

  public set size(value: vec2) {
    const gl = this.m_gl;
    this.m_size = value;

    gl.bindRenderbuffer(gl.RENDERBUFFER, this.m_colorRenderbuffer);
    gl.bindTexture(gl.TEXTURE_2D, this.m_texture);

    gl.renderbufferStorageMultisample(
      gl.RENDERBUFFER,
      this.m_samples,
      gl.RGBA8,
      value[0],
      value[1]
    );
    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      gl.RGBA,
      value[0],
      value[1],
      0,
      gl.RGBA,
      gl.UNSIGNED_BYTE,
      null
    );
  }

  bind() {
    const gl = this.m_gl;

    gl.bindFramebuffer(gl.FRAMEBUFFER, this.m_frameBuffers[FRAMEBUFFER.RENDERBUFFER]);
    gl.clearBufferfv(gl.COLOR, 0, [0.0, 0.0, 0.0, 0.0]);
  }

  render() {
    const gl = this.m_gl;
    const [width, height] = this.m_size;

    gl.bindFramebuffer(gl.READ_FRAMEBUFFER, this.m_frameBuffers[FRAMEBUFFER.RENDERBUFFER]);
    gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, this.m_frameBuffers[FRAMEBUFFER.COLORBUFFER]);
    gl.blitFramebuffer(0, 0, width, height, 0, 0, width, height, gl.COLOR_BUFFER_BIT, gl.NEAREST);

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    this.m_shaders.use('frame');

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, this.m_texture);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.m_vertexBuffer);

    this.m_shaders.setUniform('uScreenTexture', 0);
    this.m_shaders.setUniform('uResolution', this.m_size);
    this.m_shaders.setAttribute('aPosition', 2, gl.FLOAT);

    gl.clearColor(0.0, 0.0, 0.0, 0.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
  }
}
