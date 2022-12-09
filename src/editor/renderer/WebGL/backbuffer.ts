import { ShaderManager } from './ShaderManager';
import { default as fxaa } from './shaders/fxaa.glsl';

export class FrameBuffer {
  gl: WebGL2RenderingContext;
  texture: WebGLTexture;
  FRAMEBUFFER: { RENDERBUFFER: number; COLORBUFFER: number };
  frameBuffers: [WebGLFramebuffer, WebGLFramebuffer];
  _size: vec2;
  colorRenderbuffer: WebGLRenderbuffer;
  vBuffer: WebGLBuffer;
  shaders: ShaderManager;

  constructor(gl: WebGL2RenderingContext, shaders: ShaderManager, size: vec2) {
    this.gl = gl;

    shaders.create('frame', fxaa);

    // -- Init Texture
    // used for draw framebuffer storage
    var texture = gl.createTexture()!;
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, size[0], size[1], 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    gl.bindTexture(gl.TEXTURE_2D, null);

    // -- Init Frame Buffers
    var FRAMEBUFFER = {
      RENDERBUFFER: 0,
      COLORBUFFER: 1
    };
    var frameBuffers: [WebGLFramebuffer, WebGLFramebuffer] = [
      gl.createFramebuffer()!,
      gl.createFramebuffer()!
    ];
    var colorRenderbuffer = gl.createRenderbuffer()!;
    gl.bindRenderbuffer(gl.RENDERBUFFER, colorRenderbuffer);
    gl.renderbufferStorageMultisample(
      gl.RENDERBUFFER,
      gl.getParameter(gl.MAX_SAMPLES),
      gl.RGBA8,
      size[0],
      size[1]
    );

    console.log(colorRenderbuffer);

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

    // Geometry

    this.vBuffer = gl.createBuffer()!;
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vBuffer);
    gl.bufferData(
      gl.ARRAY_BUFFER,
      new Float32Array([-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0]),
      gl.STATIC_DRAW
    );

    this.FRAMEBUFFER = FRAMEBUFFER;
    this.frameBuffers = frameBuffers;
    this.colorRenderbuffer = colorRenderbuffer;
    this.texture = texture;
    this.shaders = shaders;
  }

  public set size(value: vec2) {
    const gl = this.gl;
    this._size = value;

    console.log(this.colorRenderbuffer);
    gl.bindRenderbuffer(gl.RENDERBUFFER, this.colorRenderbuffer);
    gl.bindTexture(gl.TEXTURE_2D, this.texture);

    gl.renderbufferStorageMultisample(
      gl.RENDERBUFFER,
      gl.getParameter(gl.MAX_SAMPLES),
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
    const gl = this.gl;

    gl.bindFramebuffer(gl.FRAMEBUFFER, this.frameBuffers[this.FRAMEBUFFER.RENDERBUFFER]);
    gl.clearBufferfv(gl.COLOR, 0, [1.0, 1.0, 1.0, 0.0]);
  }

  render() {
    const gl = this.gl;
    const [width, height] = this._size;

    console.log(gl.getParameter(gl.SAMPLES));

    // Blit framebuffers, no Multisample texture 2d in WebGL 2
    gl.bindFramebuffer(gl.READ_FRAMEBUFFER, this.frameBuffers[this.FRAMEBUFFER.RENDERBUFFER]);
    gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, this.frameBuffers[this.FRAMEBUFFER.COLORBUFFER]);
    gl.blitFramebuffer(0, 0, width, height, 0, 0, width, height, gl.COLOR_BUFFER_BIT, gl.NEAREST);

    // Pass 2
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    this.shaders.use('frame');

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vBuffer);

    this.shaders.setUniform('uScreenTexture', 0);
    this.shaders.setUniform('uResolution', this._size);
    this.shaders.setAttribute('aPosition', 2, gl.FLOAT);
    gl.clearColor(1.0, 1.0, 1.0, 0.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    // gl.bindFramebuffer(gl.READ_FRAMEBUFFER, this.fb[this.FRAMEBUFFER.RENDERBUFFER]);

    // gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, this.fb[this.FRAMEBUFFER.COLORBUFFER]);

    // gl.clearBufferfv(gl.COLOR, 0, [0.0, 0.0, 0.0, 0.0]);

    // gl.blitFramebuffer(0, 0, width, height, 0, 0, width, height, gl.COLOR_BUFFER_BIT, gl.LINEAR);

    // // render the top layer to the framebuffer as well
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.fb[this.FRAMEBUFFER.RENDERBUFFER]);

    // // this time render to the default buffer, which is just canvas
    // gl.bindFramebuffer(gl.READ_FRAMEBUFFER, this.fb[this.FRAMEBUFFER.RENDERBUFFER]);

    // gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, null);

    // gl.clearBufferfv(gl.COLOR, 0, [0.0, 0.0, 0.0, 0.0]);
    // gl.blitFramebuffer(0, 0, width, height, 0, 0, width, height, gl.COLOR_BUFFER_BIT, gl.LINEAR);
  }
}
