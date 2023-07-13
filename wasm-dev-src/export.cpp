#ifdef OLD
// #include "common.h"
#include "editor/editor.h"
#include "editor/scene/entity.h"
#include "editor/input/input_manager.h"
// #include "editor/text/font_manager.h"
// #include "editor/settings.h"
// #include "renderer/renderer.h"
// #include "io/svg/svg.h"

#include <stdio.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#ifdef __INTELLISENSE__
#define EMSCRIPTEN_KEEPALIVE
#endif

#include <emscripten/bind.h>

using namespace emscripten;

extern "C" {
  bool EMSCRIPTEN_KEEPALIVE on_pointer_event(
    int target, int event, int type, int button,
    float x, float y, float pressure, double time_stamp,
    bool alt, bool ctrl, bool shift
  ) {
    return Graphick::Editor::Input::InputManager::on_pointer_event(
      (Graphick::Editor::Input::InputManager::PointerTarget)target, (Graphick::Editor::Input::InputManager::PointerEvent)event, (Graphick::Editor::Input::InputManager::PointerType)type, (Graphick::Editor::Input::InputManager::PointerButton)button,
      x, y, pressure, time_stamp, alt, ctrl, shift
    );
    return true;
  }

  bool EMSCRIPTEN_KEEPALIVE on_keyboard_event(
    int event, int key,
    bool repeat, bool alt, bool ctrl, bool shift
  ) {
    return Graphick::Editor::Input::InputManager::on_keyboard_event(
      (Graphick::Editor::Input::InputManager::KeyboardEvent)event, (Graphick::Editor::Input::KeyboardKey)key,
      repeat, alt, ctrl, shift
    );
  }

  bool EMSCRIPTEN_KEEPALIVE on_resize_event(int width, int height, float dpr, int offset_x, int offset_y) {
    return Graphick::Editor::Input::InputManager::on_resize_event(width, height, dpr, offset_x, offset_y);
  }

  bool EMSCRIPTEN_KEEPALIVE on_wheel_event(int target, float delta_x, float delta_y) {
    return Graphick::Editor::Input::InputManager::on_wheel_event((Graphick::Editor::Input::InputManager::PointerTarget)target, delta_x, delta_y);
  }

  bool EMSCRIPTEN_KEEPALIVE on_clipboard_event(int event) {
    return Graphick::Editor::Input::InputManager::on_clipboard_event((Graphick::Editor::Input::InputManager::ClipboardEvent)event);
  }

  void EMSCRIPTEN_KEEPALIVE set_tool(int tool) {
    if (tool < 0 || tool >= static_cast<int>(Graphick::Editor::Input::Tool::ToolType::None)) return;

    Graphick::Editor::Input::InputManager::set_tool((Graphick::Editor::Input::Tool::ToolType)tool);
  }

  void EMSCRIPTEN_KEEPALIVE load(const char* data) {
    // Graphick::Editor::Editor::load(data);
  }

  void EMSCRIPTEN_KEEPALIVE load_font(const unsigned char* buffer, long buffer_size) {
    // FontManager::load_font(buffer, buffer_size);
  }

  void EMSCRIPTEN_KEEPALIVE load_svg(const char* svg) {
    // Graphick::SVG::parse_svg(svg);
  }

  void EMSCRIPTEN_KEEPALIVE init() {
    Graphick::Editor::Editor::init();

    Graphick::Editor::Entity test_entity = Graphick::Editor::Editor::scene().create_entity("Test Entity");
    Graphick::Renderer::Geometry::Path& path = test_entity.add_component<Graphick::Editor::PathComponent>().path;

    path.move_to({ 0.0f, 0.0f });
    path.line_to({ 20.0f, -20.0f });
    path.line_to({ 50.0f, -40.0f });
    path.line_to({ 200.0f, -50.0f });
    path.line_to({ 300.0f, -20.0f });
    path.line_to({ 350.0f, -20.0f });
    path.line_to({ 380.0f, -40.0f });
    path.line_to({ 360.0f, 20.0f });

    path.close();
  }

  void EMSCRIPTEN_KEEPALIVE shutdown() {
    Graphick::Editor::Editor::shutdown();
  }

  void EMSCRIPTEN_KEEPALIVE save() {
    // Graphick::Editor::Editor::save();
  }
}
#else

#include <emscripten.h>

#ifdef __INTELLISENSE__
#define EMSCRIPTEN_KEEPALIVE
#endif

#if 0
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#include "lib/blaze/src/DestinationImage.h"
#include "lib/blaze/src/Rasterizer.h"

static const char* CanvasElementID = "#canvas";


struct WebGLContext final {
  WebGLContext();
  ~WebGLContext();

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE Context = 0;
};


WebGLContext::WebGLContext()
{
  EmscriptenWebGLContextAttributes attr;

  emscripten_webgl_init_context_attributes(&attr);

  attr.alpha = 0;
  attr.premultipliedAlpha = 1;
  attr.antialias = 0;
  attr.depth = 0;
  attr.stencil = 0;

  Context = emscripten_webgl_create_context(CanvasElementID, &attr);

  emscripten_webgl_make_context_current(Context);
}


WebGLContext::~WebGLContext()
{
  emscripten_webgl_destroy_context(Context);
}


struct WebGLTexture final {
  void Resize(const int width, const int height);
  void UploadImage(const Blaze::uint8* data);

  GLuint ID = 0;
  int Width = 0;
  int Height = 0;
};


void WebGLTexture::Resize(const int width, const int height)
{
  if (Width != width || Height != height) {
    glDeleteTextures(1, &ID);

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    Width = width;
    Height = height;
  }
}


void WebGLTexture::UploadImage(const Blaze::uint8* data)
{
  glBindTexture(GL_TEXTURE_2D, ID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA,
    GL_UNSIGNED_BYTE, data);
}


struct WebGLProgram final {
  WebGLProgram();

  GLuint ProgramID = 0;
  GLuint VertexShaderID = 0;
  GLuint FragmentShaderID = 0;
};


static const char* pVertexSource =
"attribute vec4 position;\n"
"attribute vec2 texCoord;\n"
"varying vec2 v_texCoord;\n"
"void main() {\n"
"    gl_Position = position;\n"
"    v_texCoord = texCoord;\n"
"}\n";


static const char* pFragmentSource =
"precision mediump float;\n"
"varying vec2 v_texCoord;\n"
"uniform sampler2D texture;\n"
"void main() {\n"
"    gl_FragColor = texture2D(texture, v_texCoord);\n"
"}\n";


WebGLProgram::WebGLProgram()
{
  // Vertex.
  const GLchar* VertexSource[1] = {
      pVertexSource
  };

  const GLint VertexSourceLengths[1] = {
      static_cast<GLint>(strlen(pVertexSource))
  };

  VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(VertexShaderID, 1, VertexSource, VertexSourceLengths);
  glCompileShader(VertexShaderID);

  // Fragment.
  const GLchar* FragmentSource[1] = {
      pFragmentSource
  };

  const GLint FragmentSourceLengths[1] = {
      static_cast<GLint>(strlen(pFragmentSource))
  };

  FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(FragmentShaderID, 1, FragmentSource, FragmentSourceLengths);
  glCompileShader(FragmentShaderID);

  // Program.
  ProgramID = glCreateProgram();

  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);

  glBindAttribLocation(ProgramID, 0, "position");

  glLinkProgram(ProgramID);
}


struct WebGLTexturedQuad final {
  WebGLTexturedQuad();

  GLuint VertexObject = 0;
  GLuint IndexObject = 0;
};


WebGLTexturedQuad::WebGLTexturedQuad()
{
  // Vertices XYZUV.
  const GLfloat pVertices[] = {
    // Top left.
   -1.0,  1.0,  0.0,  0.0,  0.0,
   // Bottom left.
  -1.0, -1.0,  0.0,  0.0,  1.0,
  // Bottom right.
  1.0, -1.0,  0.0,  1.0,  1.0,
  // Top right.
  1.0,  1.0,  0.0,  1.0,  0.0
  };

  const GLushort pIndices[] = {
      0, 1, 2, 0, 2, 3
  };

  glGenBuffers(1, &VertexObject);
  glBindBuffer(GL_ARRAY_BUFFER, VertexObject);
  glBufferData(GL_ARRAY_BUFFER, SIZE_OF(pVertices), pVertices,
    GL_STATIC_DRAW);

  glGenBuffers(1, &IndexObject);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexObject);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, SIZE_OF(pIndices), pIndices,
    GL_STATIC_DRAW);
}


class MainRenderer final {
public:
  void RenderFrame();
  void TranslateCanvas(const double x, const double y);
  void SetupUserCoordinateSystem();
  void ScaleCanvas(const double delta, const double x, const double y);
  void InstallVectorImage(const Blaze::uint8* ptr, const int size);
private:
  void UpdateImageData();
  void RenderFrameGL();
  Blaze::Matrix GetMatrix() const;
private:
  WebGLContext mContext;
  WebGLTexture mTexture;
  WebGLProgram mProgram;
  WebGLTexturedQuad mTexturedQuad;

  Blaze::DestinationImage<Blaze::TileDescriptor_16x8> mImage;
  Blaze::VectorImage mVectorImage;
  Blaze::Matrix mCoordinateSystemMatrix;
  Blaze::FloatPoint mTranslation;
  double mScale = 1;
};


void MainRenderer::RenderFrame()
{
  emscripten_webgl_make_context_current(mContext.Context);

  UpdateImageData();

  SetupUserCoordinateSystem();

  mImage.DrawImage(mVectorImage, GetMatrix());

  RenderFrameGL();
}


void MainRenderer::TranslateCanvas(const double x, const double y)
{
  mTranslation.X += x;
  mTranslation.Y += y;
}


void MainRenderer::ScaleCanvas(const double delta, const double x,
  const double y)
{
  const double zoom = Blaze::Clamp(mScale * delta, 0.001, 100000.0);

  if (mScale != zoom) {
    const double dd = (zoom - mScale) / mScale;
    const Blaze::FloatPoint pn = mCoordinateSystemMatrix.Inverse().Map(x, y);

    mTranslation.X += (mTranslation.X - pn.X) * dd;
    mTranslation.Y += (mTranslation.Y - pn.Y) * dd;
    mScale = zoom;
  }
}


Blaze::Matrix MainRenderer::GetMatrix() const
{
  Blaze::Matrix m = Blaze::Matrix::CreateScale(mScale, mScale);

  m.PreTranslate(mTranslation.X, mTranslation.Y);

  m.PreMultiply(mCoordinateSystemMatrix);

  return m;
}


void MainRenderer::InstallVectorImage(const Blaze::uint8* ptr, const int size)
{
  mVectorImage.Parse(ptr, size);

  mTranslation.X = 0;
  mTranslation.Y = 0;
  mScale = 1;
}


void MainRenderer::SetupUserCoordinateSystem()
{
  const Blaze::IntRect bounds = mVectorImage.GetBounds();

  const double mx = double(bounds.MaxX - bounds.MinX) / 2.0;
  const double my = double(bounds.MaxY - bounds.MinY) / 2.0;

  mCoordinateSystemMatrix = Blaze::Matrix::CreateTranslation(
    (double(mImage.GetImageWidth()) / 2.0) - mx,
    (double(mImage.GetImageHeight()) / 2.0) - my);
}


void MainRenderer::RenderFrameGL()
{
  mTexture.Resize(mImage.GetImageWidth(), mImage.GetImageHeight());

  mTexture.UploadImage(mImage.GetImageData());

  glUseProgram(mProgram.ProgramID);
  glBindBuffer(GL_ARRAY_BUFFER, mTexturedQuad.VertexObject);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTexturedQuad.IndexObject);

  const GLint positionLocation = glGetAttribLocation(
    mProgram.ProgramID, "position");
  const GLint textureCoordinateLocation = glGetAttribLocation(
    mProgram.ProgramID, "texCoord");
  const GLint textureLocation = glGetUniformLocation(
    mProgram.ProgramID, "texture");

  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glViewport(0, 0, mImage.GetImageWidth(), mImage.GetImageHeight());

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE,
    5 * SIZE_OF(GLfloat), 0);
  glVertexAttribPointer(textureCoordinateLocation, 2, GL_FLOAT, GL_FALSE,
    5 * SIZE_OF(GLfloat), reinterpret_cast<GLvoid*>((3 * SIZE_OF(GLfloat))));

  glUniform1i(textureLocation, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mTexture.ID);

  glEnableVertexAttribArray(positionLocation);
  glEnableVertexAttribArray(textureCoordinateLocation);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}


void MainRenderer::UpdateImageData()
{
  int h = 0;
  int v = 0;

  emscripten_webgl_get_drawing_buffer_size(mContext.Context, &h, &v);

  const Blaze::IntSize imageSize = mImage.UpdateSize(Blaze::IntSize{
      h, v
    });

  mImage.ClearImage();
}


static MainRenderer* MR = nullptr;
static bool NeedsRendering = true;


static void AnimationFrame()
{
  if (NeedsRendering) {
    MR->RenderFrame();

    NeedsRendering = false;
  }
}
#endif

#include "renderer/renderer.h"
#include "editor/editor.h"
#include "editor/input/input_manager.h"
#include "io/svg/svg.h"

extern "C" {

  bool EMSCRIPTEN_KEEPALIVE on_pointer_event(
    int target, int event, int type, int button,
    float x, float y, float pressure, double time_stamp,
    bool alt, bool ctrl, bool shift
  ) {
    return Graphick::Editor::Input::InputManager::on_pointer_event(
      (Graphick::Editor::Input::InputManager::PointerTarget)target, (Graphick::Editor::Input::InputManager::PointerEvent)event, (Graphick::Editor::Input::InputManager::PointerType)type, (Graphick::Editor::Input::InputManager::PointerButton)button,
      x, y, pressure, time_stamp, alt, ctrl, shift
    );
    return true;
  }

  bool EMSCRIPTEN_KEEPALIVE on_keyboard_event(
    int event, int key,
    bool repeat, bool alt, bool ctrl, bool shift
  ) {
    return Graphick::Editor::Input::InputManager::on_keyboard_event(
      (Graphick::Editor::Input::InputManager::KeyboardEvent)event, (Graphick::Editor::Input::KeyboardKey)key,
      repeat, alt, ctrl, shift
    );
  }

  bool EMSCRIPTEN_KEEPALIVE on_resize_event(int width, int height, float dpr, int offset_x, int offset_y) {
    return Graphick::Editor::Input::InputManager::on_resize_event(width, height, dpr, offset_x, offset_y);
  }

  bool EMSCRIPTEN_KEEPALIVE on_wheel_event(int target, float delta_x, float delta_y) {
    return Graphick::Editor::Input::InputManager::on_wheel_event((Graphick::Editor::Input::InputManager::PointerTarget)target, delta_x, delta_y);
  }

  bool EMSCRIPTEN_KEEPALIVE on_clipboard_event(int event) {
    return Graphick::Editor::Input::InputManager::on_clipboard_event((Graphick::Editor::Input::InputManager::ClipboardEvent)event);
  }

  void EMSCRIPTEN_KEEPALIVE set_tool(int tool) {
    if (tool < 0 || tool >= static_cast<int>(Graphick::Editor::Input::Tool::ToolType::None)) return;

    Graphick::Editor::Input::InputManager::set_tool((Graphick::Editor::Input::Tool::ToolType)tool);
  }

  void EMSCRIPTEN_KEEPALIVE load(const char* data) {
    // Graphick::Editor::Editor::load(data);
  }

  void EMSCRIPTEN_KEEPALIVE load_font(const unsigned char* buffer, long buffer_size) {
    // FontManager::load_font(buffer, buffer_size);
  }

  void EMSCRIPTEN_KEEPALIVE load_svg(const char* svg) {
    Graphick::io::svg::parse_svg(svg);
  }

  void EMSCRIPTEN_KEEPALIVE save() {
    // Graphick::Editor::Editor::save();
  }

  void EMSCRIPTEN_KEEPALIVE RenderFrame()
  {
    Graphick::Editor::Editor::render();
    // MR->RenderFrame();
  }


  void EMSCRIPTEN_KEEPALIVE TranslateCanvas(const double x, const double y)
  {
    // MR->TranslateCanvas(x, y);

    // NeedsRendering = true;
  }


  void EMSCRIPTEN_KEEPALIVE ScaleCanvas(const double delta, const double x,
    const double y)
  {
    // MR->ScaleCanvas(delta, x, y);

    // NeedsRendering = true;
  }


  void EMSCRIPTEN_KEEPALIVE InstallVectorImage(const uintptr_t ptr, const size_t size)
  {
    Graphick::Renderer::Renderer::upload_vector_image(reinterpret_cast<const uint8_t*>(ptr), static_cast<int>(size));
    // MR->InstallVectorImage(reinterpret_cast<const Blaze::uint8*>(ptr),
      // static_cast<int>(size));
  }


  static EM_BOOL EMSCRIPTEN_KEEPALIVE DoFrame(double time, void* userData) {
    // AnimationFrame();

    return EM_TRUE;
  }


  void EMSCRIPTEN_KEEPALIVE init() {
    Graphick::Editor::Editor::init();
  }

  void EMSCRIPTEN_KEEPALIVE shutdown() {
    Graphick::Editor::Editor::shutdown();
  }

  // int EMSCRIPTEN_KEEPALIVE main()
  // {
  //   MR = new MainRenderer();

  //   emscripten_request_animation_frame_loop(DoFrame, 0);

  //   return 1;
  // }

}

#endif