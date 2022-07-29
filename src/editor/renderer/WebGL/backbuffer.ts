import { ShaderManager } from './shaderManager';
import { default as framebuffer } from './shaders/framebuffer.glsl';

class BackBuffer {
  private m_gl: WebGL2RenderingContext;
  private m_shaders: ShaderManager;
  private m_size: vec2;

  public frameBuffer: WebGLFramebuffer;
  public renderBuffer: WebGLRenderbuffer;
  public texture: WebGLTexture;
  public vertexBuffer: WebGLBuffer;

  constructor(gl: WebGL2RenderingContext, shaders: ShaderManager, size: vec2) {
    this.m_gl = gl;
    this.m_shaders = shaders;
    this.m_size = size;

    this.m_shaders.create('frame', framebuffer, false);

    // Buffers

    this.frameBuffer = gl.createFramebuffer()!;
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.frameBuffer);

    this.renderBuffer = gl.createRenderbuffer()!;
    gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderBuffer);

    this.texture = gl.createTexture()!;
    gl.bindTexture(gl.TEXTURE_2D, this.texture);

    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      gl.RGB,
      size[0],
      size[1],
      0,
      gl.RGB,
      gl.UNSIGNED_BYTE,
      null
    );

    gl.renderbufferStorage(
      gl.RENDERBUFFER,
      gl.DEPTH_COMPONENT16,
      size[0],
      size[1]
    );
    gl.framebufferTexture2D(
      gl.FRAMEBUFFER,
      gl.COLOR_ATTACHMENT0,
      gl.TEXTURE_2D,
      this.texture,
      0
    );
    gl.framebufferRenderbuffer(
      gl.FRAMEBUFFER,
      gl.DEPTH_ATTACHMENT,
      gl.RENDERBUFFER,
      this.renderBuffer
    );

    // Geometry

    this.vertexBuffer = gl.createBuffer()!;
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
    gl.bufferData(
      gl.ARRAY_BUFFER,
      new Float32Array([-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0]),
      gl.STATIC_DRAW
    );

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    gl.bindTexture(gl.TEXTURE_2D, null);
  }

  public set size(value: vec2) {
    const gl = this.m_gl;
    this.m_size = value;

    gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderBuffer);
    gl.bindTexture(gl.TEXTURE_2D, this.texture);

    gl.renderbufferStorage(
      gl.RENDERBUFFER,
      gl.DEPTH_COMPONENT16,
      value[0],
      value[1]
    );
    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      gl.RGB,
      value[0],
      value[1],
      0,
      gl.RGB,
      gl.UNSIGNED_BYTE,
      null
    );

    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    gl.bindTexture(gl.TEXTURE_2D, null);
  }

  public render() {
    const gl = this.m_gl;

    this.m_shaders.use('frame');

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);

    this.m_shaders.setUniform('uScreenTexture', 0);
    this.m_shaders.setUniform('uResolution', this.m_size);
    this.m_shaders.setAttribute('aPosition', 2, gl.FLOAT);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
  }
}

export default BackBuffer;
