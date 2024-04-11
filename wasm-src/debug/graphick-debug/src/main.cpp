#include "wasm-src/editor/editor.h"
#include "wasm-src/editor/scene/entity.h"
#include "wasm-src/editor/input/input_manager.h"

#include "wasm-src/io/svg/svg.h"

#include "wasm-src/utils/console.h"

#include "wasm-src/renderer/geometry/path.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <stdio.h>

struct PointerState {
  Graphick::vec2 position;
  Graphick::Editor::Input::InputManager::PointerButton button;
  bool alt;
  bool ctrl;
  bool shift;
};

static PointerState pointer_state;
static float dpr;

static void cursor_position_callback(GLFWwindow* window, double x, double y) {
  OPTICK_EVENT();

  pointer_state.position = Graphick::vec2{ (float)x, (float)y } / dpr;

  Graphick::Editor::Input::InputManager::on_pointer_event(
    Graphick::Editor::Input::InputManager::PointerTarget::Canvas, Graphick::Editor::Input::InputManager::PointerEvent::Move,
    Graphick::Editor::Input::InputManager::PointerType::Mouse, pointer_state.button,
    pointer_state.position.x, pointer_state.position.y, 1.0f, glfwGetTime() * 1000,
    pointer_state.alt, pointer_state.ctrl, pointer_state.shift
  );
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  pointer_state.button = (Graphick::Editor::Input::InputManager::PointerButton)button;

  Graphick::Editor::Input::InputManager::on_pointer_event(
    Graphick::Editor::Input::InputManager::PointerTarget::Canvas, action == GLFW_PRESS ? Graphick::Editor::Input::InputManager::PointerEvent::Down : Graphick::Editor::Input::InputManager::PointerEvent::Up,
    Graphick::Editor::Input::InputManager::PointerType::Mouse, pointer_state.button,
    pointer_state.position.x, pointer_state.position.y, 1.0f, glfwGetTime() * 1000,
    pointer_state.alt, pointer_state.ctrl, pointer_state.shift
  );
}

static void window_resize_callback(GLFWwindow* window, int width, int height) {
  Graphick::Editor::Input::InputManager::on_resize_event((int)(width / dpr), (int)(height / dpr), dpr, 0, 0);
}

static void scroll_callback(GLFWwindow* window, double delta_x, double delta_y) {
  Graphick::Editor::Input::InputManager::on_wheel_event(Graphick::Editor::Input::InputManager::PointerTarget::Canvas, -(float)delta_x, -(float)delta_y, pointer_state.ctrl);
}

static void cursor_enter_callback(GLFWwindow* window, int entered) {
  Graphick::Editor::Input::InputManager::on_pointer_event(
    Graphick::Editor::Input::InputManager::PointerTarget::Canvas, entered ? Graphick::Editor::Input::InputManager::PointerEvent::Enter : Graphick::Editor::Input::InputManager::PointerEvent::Leave,
    Graphick::Editor::Input::InputManager::PointerType::Mouse, pointer_state.button,
    pointer_state.position.x, pointer_state.position.y, 1.0f, glfwGetTime() * 1000,
    pointer_state.alt, pointer_state.ctrl, pointer_state.shift
  );
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers) {
  pointer_state.alt = modifiers & GLFW_MOD_ALT;
  pointer_state.ctrl = modifiers & GLFW_MOD_CONTROL;
  pointer_state.shift = modifiers & GLFW_MOD_SHIFT;

  Graphick::Editor::Input::InputManager::on_keyboard_event(
    action == GLFW_RELEASE ? Graphick::Editor::Input::InputManager::KeyboardEvent::Up : Graphick::Editor::Input::InputManager::KeyboardEvent::Down, (Graphick::Editor::Input::KeyboardKey)key, action == GLFW_REPEAT,
    pointer_state.alt, pointer_state.ctrl, pointer_state.shift
  );
}

int main() {
  GLFWwindow* window;

  if (!glfwInit()) {
    printf("Failed to initialize GLFW\n");
    return -1;
  }

  int width = 800;
  int height = 600;

  window = glfwCreateWindow(width, height, "Graphick", nullptr, nullptr);
  glfwSetWindowPos(window, 0, 30);

  if (!window) {
    printf("Failed to create window\n");
    glfwTerminate();
    return -1;
  }

  float x_scale, y_scale;
  glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &x_scale, &y_scale);

  dpr = (x_scale + y_scale) / 2.0f;

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetWindowSizeCallback(window, window_resize_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetCursorEnterCallback(window, cursor_enter_callback);
  glfwSetKeyCallback(window, key_callback);

  glDisable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH);

  Graphick::Editor::Editor::init();
  Graphick::Editor::Input::InputManager::on_resize_event((int)(width / dpr), (int)(height / dpr), dpr, 0, 0);

// #define TIGER
#define OBJECTS

#ifdef TIGER
  // std::ifstream ifs("res\\test.svg");
  std::ifstream ifs("res\\Ghostscript_Tiger.svg");
  std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  Graphick::io::svg::parse_svg(content);
#elif defined(OBJECTS)
  Graphick::Renderer::Geometry::Path path1;

  // Graphick::Editor::Entity test_entity1 = Graphick::Editor::Editor::scene().create_element("Test Entity 1");

  // auto& path_component = test_entity.get_component<Graphick::Editor::PathComponent>();
  // auto& path = test_entity.get_component<Graphick::Editor::PathComponent>().data;
  // Graphick::Renderer::Geometry::Path& path = test_entity.get_component<Graphick::Editor::PathComponent>().data;
  // Graphick::Renderer::Geometry::Path& path1 = test_entity1.get_component<Graphick::Editor::PathComponent>().path;

  // path.move_to({ 360.0f, 20.0f });
  // path1.move_to({ 0.0f, 0.0f });
  // path1.line_to({ 0.0f, -50.0f });
  // path1.line_to({ 50.0f, -40.0f });
  // // path.line_to({ 200.0f, -50.0f });
  // // path.line_to({ 300.0f, -20.0f });
  // // path.line_to({ 350.0f, -20.0f });
  // // path.line_to({ 380.0f, -40.0f });
  // path1.close();

  // path1.move_to({ 100.0f, 100.0f });
  // path1.cubic_to({ 100.0f, 100.0f }, { 300.0f, 100.0f }, { 300.0f, 100.0f });

  // path1.move_to({ 100.0f, 100.0f });
  // path1.line_to({ 200.0f, 200.0f });
  // path1.line_to({ 80.0f, 190.0f });
  // path1.close();

  path1.move_to({ 0.0f, 0.0f });
  path1.quadratic_to({ 100.0f, 100.0f }, { 200.0f, 000.0f });
  // path1.quadratic_to({ 100.0f, -100.0f }, { 0.0f, 0.0f });
  // path1.close();

  // path1.move_to({ 300.0f, 100.0f });
  // path1.cubic_to({ 300.0f, 200.0f }, { 320.0f, 150.0f }, { 400.0f, 100.0f });

  // path1.circle({ 100.0f, 100.0f }, 50.0f);
  // path1.move_to({ 300.0f, 100.0f });
  // path1.cubic_to({ 300.0f, 200.0f }, { 400.0f, 200.0f }, { 400.0f, 100.0f });
  // path1.cubic_to({ 400.0f, 0.0f }, { 300.0f, 0.0f }, { 300.0f, 100.0f });
  // path1.line_to({ 350.0f, 200.0f });
  // path1.line_to({ 250.0f, 200.0f });
  // path1.close();

  // path1.move_to({ 500.0f, 100.0f });
  // path1.line_to({ 550.0f, 200.0f });
  // path1.line_to({ 450.0f, 200.0f });
  // path1.close();


  // TODO: handle case of multiple contours overlapping
  // TODO: fix outlines when zooming in
  // path1.move_to({ 230.0f, 324.0f });
  // path1.cubic_to({ 541.0f, 358.0f }, { 351.0f, 160.0f }, { 325.0f, 391.0f }); // Handled

  // path1.move_to({ 221.0f, 718.0f });
  // path1.cubic_to({ 620.0f, 450.0f }, { 190.0f, 140.0f }, { 518.0f, 243.0f }); // Handled

  // path1.move_to({ 295.0f, 343.0f });
  // path1.cubic_to({ 436.0f, 203.0f }, { 540.0f, 323.0f }, { 540.0f, 323.0f }); // Handled

  Graphick::Editor::Entity test_entity = Graphick::Editor::Editor::scene().create_element(path1);

  /* Stroking Robustness */

  // path.move_to({ 230.0f, 324.0f });
  // path.cubic_to({ 541.0f, 358.0f }, { 351.0f, 160.0f }, { 325.0f, 391.0f }); // Handled

  // path.move_to({ 221.0f, 718.0f });
  // path.cubic_to({ 620.0f, 450.0f }, { 190.0f, 140.0f }, { 518.0f, 243.0f }); // Handled

  // path.move_to({ 295.0f, 343.0f });
  // path.cubic_to({ 436.0f, 203.0f }, { 307.0f, 221.0f }, { 540.0f, 323.0f }); // Handled

  // path.move_to({ 50.0f, 200.0f });
  // path.cubic_to({ -150.0f, 100.0f }, { -50.0f, 100.0f }, { -50.0f, 200.0f }); // Handled

  // path.move_to({ 50.0f, 200.0f });
  // path.cubic_to({ -150.0f, 100.0f }, { -37.0f, 112.0f }, { -50.0f, 200.0f }); // Handled

  // path.move_to({ 0.0f, 0.0f });
  // path.cubic_to({ 110.0f, 100.0f }, { -10.0f, 100.0f }, { 100.0f, 0.0f }); // Handled

  // path.move_to({ 0.0f, 0.0f });
  // path.cubic_to({ 101.0f, 100.0f }, { -1.0f, 100.0f }, { 100.0f, 0.0f }); // Handled

  // path.move_to({ 0.0f, 0.0f });
  // path.cubic_to({ 100.0f, 100.0f }, { 0.0f, 100.0f }, { 100.0f, 0.0f }); // Handled

  // path.move_to({ 0.0f, 0.0f });
  // path.cubic_to({ 10.0f, 60.0f }, { 0.0f, 60.0f }, { 10.0f, 50.0f }); // High Curvature Endpoint (naive)

  // path.move_to({ 357.188f, 170.417f });
  // path.cubic_to({ 360.313f, 175.417f }, { 1304.06f, -507.917f }, { 0.0f, 0.0f }); // High Curvature Endpoint (naive)

  // path.move_to({ 75.8624f, 74.2385f });
  // path.cubic_to({ 272.016f, 272.517f }, { 39.1216f, 36.2832f }, { 200.0f, 200.0f }); // Handled

  // path.move_to({ 129.012f, 295.262f });
  // path.cubic_to({ 129.113f, 295.551f }, { 116.507f, 268.718f }, { 117.396f, 270.102f }); // High Curvature Endpoint (naive)

  // path.move_to({ 0.0f, 150.0f });
  // path.cubic_to({ -60.0f, 250.0f }, { -10.0f, 350.0f }, { 150.0f, 450.0f }); // Handled

  // path.move_to({ 0.0f, 0.0f });
  // path.cubic_to({ 33.0f, 66.0f }, { 66.0f, 66.0f }, { 100.0f, 0.0f }); // Handled

  // path.move_to({ 100.0f, 100.0f });
  // path.cubic_to({ 203.243f, 170.858f }, { 100.0f, 100.0f }, { 200.0f, 200.0f }); // Handled

  // path.move_to({ 100.0f, 100.0f });
  // path.cubic_to({ 200.0f, 200.0f }, { 529.0f, 160.0f }, { 400.0f, 400.0f }); // Handled

  // path.move_to({ 0.0f, 0.0f });
  // path.cubic_to({ -2.0f, 10.0f }, { -2.0f, 90.0f }, { 0.0f, 100.0f });
  // path.cubic_to({ 0.0f, 100.0f }, { 210.0f, -300.0f }, { 200.0f, 100.0f });
  // path.line_to({ 230.0f, 70.0f });
  // path.cubic_to({ 225.0f, 73.0f }, { 173.0f, 0.0f }, { 170.0f, -60.0f });
  // path.cubic_to({ 80.0f, 0.0f }, { 500.0f, 70.0f }, { 0.0f, 0.0f });
  // path.close();

  // path.move_to({ 0.0f, 0.0f });
  // path.cubic_to({ 0.0f, 0.0f }, { -47.0f, -36.0f }, { -50.0f, -40.0f });
  // path.cubic_to({ -50.0f, -40.0f }, { -70.0f, -85.0f }, { -110.0f, -85.0f });
  // path.cubic_to({ -110.0f, -85.0f }, { -170.0f, -85.0f }, { -220.0f, -45.0f });

  // path1.move_to({ 100.0f, 0.0f });
  // path1.line_to({ 20.0f, -20.0f });
  // path1.line_to({ 80.0f, 0.0f });
  // path1.line_to({ 200.0f, -50.0f });
  // path1.line_to({ 100.0f, -20.0f });
  // path1.line_to({ 350.0f, -50.0f });
  // path1.line_to({ 320.0f, -40.0f });
  // path1.line_to({ 360.0f, 260.0f });

  // path1.close();

  test_entity.add_component<Graphick::Editor::FillComponent>(Graphick::vec4{ 0.8f, 0.3f, 0.3f, 1.0f });
  Graphick::Editor::StrokeComponent stroke = test_entity.add_component<Graphick::Editor::StrokeComponent>(Graphick::vec4{ 0.93f, 0.64f, 0.74f, 1.0f });

  const_cast<Graphick::Editor::StrokeComponent::Data*>(&stroke.stroke_TEMP())->width = 20.0f;
  const_cast<Graphick::Editor::StrokeComponent::Data*>(&stroke.stroke_TEMP())->cap = Graphick::Renderer::LineCap::Round;
  const_cast<Graphick::Editor::StrokeComponent::Data*>(&stroke.stroke_TEMP())->join = Graphick::Renderer::LineJoin::Round;

  // test_entity1.add_component<Graphick::Editor::FillComponent>(Graphick::vec4{ 1.0f, 0.3f, 0.3f, 1.0f });
  // test_entity1.add_component<Graphick::Editor::StrokeComponent>(Graphick::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
#endif

  while (!glfwWindowShouldClose(window)) {
    OPTICK_FRAME("MainThread");
    GK_FRAME("MainThread");

    glfwPollEvents();

    Graphick::Editor::Editor::render(true);

    glfwSwapBuffers(window);
  }

  Graphick::Editor::Editor::shutdown();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
