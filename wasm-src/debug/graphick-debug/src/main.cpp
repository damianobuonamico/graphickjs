#include "wasm-src/geom/path.h"

#include "wasm-src/editor/editor.h"
#include "wasm-src/editor/input/input_manager.h"
#include "wasm-src/editor/scene/entity.h"

#include "wasm-src/io/svg/svg.h"

#include "wasm-src/utils/console.h"

#include "wasm-src/geom/path.h"

#include "wasm-src/math/math.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <fstream>
#include <stdio.h>

struct PointerState {
  graphick::vec2 position;
  graphick::editor::input::InputManager::PointerButton button;
  bool alt;
  bool ctrl;
  bool shift;
};

static PointerState pointer_state;
static float dpr;

static void cursor_position_callback(GLFWwindow* window, double x, double y) {
  OPTICK_EVENT();

  pointer_state.position = graphick::vec2{(float)x, (float)y} / dpr;

  graphick::editor::input::InputManager::on_pointer_event(
    graphick::editor::input::InputManager::PointerTarget::Canvas,
    graphick::editor::input::InputManager::PointerEvent::Move,
    graphick::editor::input::InputManager::PointerType::Mouse,
    pointer_state.button,
    pointer_state.position.x,
    pointer_state.position.y,
    1.0f,
    glfwGetTime() * 1000,
    pointer_state.alt,
    pointer_state.ctrl,
    pointer_state.shift
  );
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  pointer_state.button = (graphick::editor::input::InputManager::PointerButton)button;

  graphick::editor::input::InputManager::on_pointer_event(
    graphick::editor::input::InputManager::PointerTarget::Canvas,
    action == GLFW_PRESS ? graphick::editor::input::InputManager::PointerEvent::Down
                         : graphick::editor::input::InputManager::PointerEvent::Up,
    graphick::editor::input::InputManager::PointerType::Mouse,
    pointer_state.button,
    pointer_state.position.x,
    pointer_state.position.y,
    1.0f,
    glfwGetTime() * 1000,
    pointer_state.alt,
    pointer_state.ctrl,
    pointer_state.shift
  );
}

static void window_resize_callback(GLFWwindow* window, int width, int height) {
  graphick::editor::input::InputManager::on_resize_event((int)(width / dpr), (int)(height / dpr), dpr, 0, 0);
}

static void scroll_callback(GLFWwindow* window, double delta_x, double delta_y) {
  graphick::editor::input::InputManager::on_wheel_event(
    graphick::editor::input::InputManager::PointerTarget::Canvas,
    -(float)delta_x * 0.75f,
    -(float)delta_y * 0.75f,
    pointer_state.ctrl
  );
}

static void cursor_enter_callback(GLFWwindow* window, int entered) {
  graphick::editor::input::InputManager::on_pointer_event(
    graphick::editor::input::InputManager::PointerTarget::Canvas,
    entered ? graphick::editor::input::InputManager::PointerEvent::Enter
            : graphick::editor::input::InputManager::PointerEvent::Leave,
    graphick::editor::input::InputManager::PointerType::Mouse,
    pointer_state.button,
    pointer_state.position.x,
    pointer_state.position.y,
    1.0f,
    glfwGetTime() * 1000,
    pointer_state.alt,
    pointer_state.ctrl,
    pointer_state.shift
  );
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers) {
  pointer_state.alt = modifiers & GLFW_MOD_ALT;
  pointer_state.ctrl = modifiers & GLFW_MOD_CONTROL;
  pointer_state.shift = modifiers & GLFW_MOD_SHIFT;

  graphick::editor::input::InputManager::on_keyboard_event(
    action == GLFW_RELEASE ? graphick::editor::input::InputManager::KeyboardEvent::Up
                           : graphick::editor::input::InputManager::KeyboardEvent::Down,
    (graphick::editor::input::KeyboardKey)key,
    action == GLFW_REPEAT,
    pointer_state.alt,
    pointer_state.ctrl,
    pointer_state.shift
  );
}

int main() {
  graphick::geom::Path<float> pppp;

  GLFWwindow* window;

  if (!glfwInit()) {
    printf("Failed to initialize GLFW\n");
    return -1;
  }

  int width = 800;
  int height = 600;

  window = glfwCreateWindow(width, height, "graphick", nullptr, nullptr);
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
  glfwSwapInterval(1);    // Enable vsync
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetWindowSizeCallback(window, window_resize_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetCursorEnterCallback(window, cursor_enter_callback);
  glfwSetKeyCallback(window, key_callback);

  glDisable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH);

  graphick::editor::Editor::init();
  graphick::editor::input::InputManager::on_resize_event((int)(width / dpr), (int)(height / dpr), dpr, 0, 0);

#define TIGER
  // #define OBJECTS

#ifdef TIGER
  // std::ifstream ifs("res\\test4.svg");
  std::ifstream ifs("res\\Ghostscript_Tiger.svg");
  std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  graphick::io::svg::parse_svg(content);
#elif defined(OBJECTS)
  graphick::geom::path path1;

  // graphick::editor::Entity test_entity1 = graphick::editor::Editor::scene().create_element("Test Entity 1");

  // auto& path_component = test_entity.get_component<graphick::editor::PathComponent>();
  // auto& path = test_entity.get_component<graphick::editor::PathComponent>().data;
  // graphick::geom::path& path = test_entity.get_component<graphick::editor::PathComponent>().data;
  // graphick::geom::path& path1 = test_entity1.get_component<graphick::editor::PathComponent>().path;

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

  // path1.move_to({ 0.0f, 0.0f });
  // path1.quadratic_to({ 100.0f, 100.0f }, { 200.0f, 000.0f });
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
  // path1.cubic_to({ 541.0f, 358.0f }, { 351.0f, 160.0f }, { 325.0f, 391.0f }); // Not Handled

  // path1.move_to({ 221.0f, 718.0f });
  // path1.cubic_to({ 620.0f, 450.0f }, { 190.0f, 140.0f }, { 518.0f, 243.0f }); // Not Handled

  // path1.move_to({ 295.0f, 343.0f });
  // path1.cubic_to({ 436.0f, 203.0f }, { 540.0f, 323.0f }, { 540.0f, 323.0f }); // Not Handled

  /* Stroking Robustness */

  using graphick::vec2;

  std::vector<std::array<vec2, 4>> tests = {
    std::array<vec2, 4>{vec2{230.0f, 324.0f}, {541.0f, 358.0f}, {351.0f, 160.0f}, {325.0f, 391.0f}},    // Handled
    std::array<vec2, 4>{vec2{221.0f, 718.0f}, {620.0f, 450.0f}, {190.0f, 140.0f}, {518.0f, 243.0f}},    // Handled
    std::array<vec2, 4>{vec2{295.0f, 343.0f}, {436.0f, 203.0f}, {307.0f, 221.0f}, {540.0f, 323.0f}},    // Handled
    std::array<vec2, 4>{vec2{50.0f, 200.0f}, {-150.0f, 100.0f}, {-50.0f, 100.0f}, {-50.0f, 200.0f}},    // Not Handled
    std::array<vec2, 4>{vec2{50.0f, 200.0f}, {-150.0f, 100.0f}, {-37.0f, 112.0f}, {-50.0f, 200.0f}},    // Not Handled (small
                                                                                                        // error)
    std::array<vec2, 4>{vec2{0.0f, 0.0f}, {110.0f, 100.0f}, {-10.0f, 100.0f}, {100.0f, 0.0f}},          // Handled
    std::array<vec2, 4>{vec2{0.0f, 0.0f}, {101.0f, 100.0f}, {-1.0f, 100.0f}, {100.0f, 0.0f}},           // Not Handled
    std::array<vec2, 4>{vec2{0.0f, 0.0f}, {100.0f, 100.0f}, {0.0f, 100.0f}, {100.0f, 0.0f}},            // Not Handled
    std::array<vec2, 4>{vec2{0.0f, 0.0f}, {10.0f, 60.0f}, {0.0f, 60.0f}, {10.0f, 50.0f}},    // High Curvature Endpoint (naive)
    std::array<vec2, 4>{vec2{357.188f, 170.417f}, {360.313f, 175.417f}, {1304.06f, -507.917f}, {0.0f, 0.0f}},    // High Curvature
                                                                                                                 // Endpoint
                                                                                                                 // (naive)
    std::array<vec2, 4>{vec2{75.8624f, 74.2385f}, {272.016f, 272.517f}, {39.1216f, 36.2832f}, {200.0f, 200.0f}},    // Not Handled
    std::array<vec2, 4>{
      vec2{129.012f, 295.262f},
      {129.113f, 295.551f},
      {116.507f, 268.718f},
      {117.396f, 270.102f}
    },                                                                                                      // High
                                                                                                            // Curvature
                                                                                                            // Endpoint
                                                                                                            // (naive)
    std::array<vec2, 4>{vec2{0.0f, 150.0f}, {-60.0f, 250.0f}, {-10.0f, 350.0f}, {150.0f, 450.0f}},          // Not Handled
    std::array<vec2, 4>{vec2{0.0f, 0.0f}, {33.0f, 66.0f}, {66.0f, 66.0f}, {100.0f, 0.0f}},                  // Not Handled
    std::array<vec2, 4>{vec2{100.0f, 100.0f}, {203.243f, 170.858f}, {100.0f, 100.0f}, {200.0f, 200.0f}},    // Not Handled
    std::array<vec2, 4>{vec2{100.0f, 100.0f}, {200.0f, 200.0f}, {529.0f, 160.0f}, {400.0f, 400.0f}}         // Not Handled
  };

  for (size_t i = 0; i < tests.size(); i++) {
    graphick::geom::path path;

    // const graphick::vec2 delta = {0.0f, 0.0f};
    const graphick::vec2 delta = {(i / 5) * 300.0f, (i % 5) * 300.0f};

    // const auto& [p, in_p1, in_p2, out_p1, out_p2] = graphick::math::split_bezier(tests[i][0], tests[i][1], tests[i][2],
    // tests[i][3], 0.5f);

    // path.move_to(delta + tests[i][0]);
    // path.cubic_to(delta + in_p1, delta + in_p2, delta + p);
    // path.cubic_to(delta + out_p1, delta + out_p2, delta + tests[i][3]);

    // path.move_to(vec2(100.0f, 0.0f));
    // path.line_to(vec2(100.0f, 100.0f));
    // path.cubic_to(vec2(50.0f, 100.0f), vec2(250.0f, 100.0f), vec2(200.0f, 100.0f));
    // path.line_to(vec2(200.0f, 200.0f));

    path.move_to(vec2(0.0f, 0.0f));
    // path.cubic_to(vec2(150.0f, 200.0f), vec2(300.0f, 300.0f), vec2(350.0f, 100.0f));
    // path.quadratic_to(vec2(150.0f, 150.0f), vec2(300.0f, 100.0f));

    path.line_to(vec2(100.0f, 100.0f));
    // path.line_to(vec2(120.0f, 500.0f / 3.0f));
    path.line_to(vec2(0.0f, 200.0f));
    // path.line_to(vec2(0.0f, 400.0f));
    // path.line_to(vec2(350.0f, 200.0f));
    // path.line_to(vec2(-50.0f, 100.0f));
    path.close();

    // path.move_to(vec2(243.839981f, 37.4800110f));
    // path.cubic_to(vec2(243.839981f, 37.4800110f), vec2(252.350464f, 138.025497f), vec2(252.350464f, 138.025528f));

    // path.cubic_to(vec2(252.350464f, 138.025528f), vec2(252.350464f, 138.025528f), vec2(252.350464f, 138.025528f));

    // path.cubic_to(vec2(252.350464f, 138.025528f), vec2(265.729980f, 26.1800385f), vec2(265.729980f, 26.1800385f));

    // path.cubic_to(vec2(265.729980f, 26.1800117f), vec2(265.729980f, 26.1800117f), vec2(265.729980f, 26.1800117f));

    // path.move_to(vec2(265.729980f, 26.1800117f));
    // path.cubic_to(vec2(263.023438f, 31.8182659f), vec2(259.879883f, 31.8209515f), vec2(256.249481f, 31.8240528f));

    // path.cubic_to(vec2(252.617035f, 31.8271561f), vec2(248.497192f, 31.8306770f), vec2(243.839981f, 37.4800110f));

    // path.cubic_to(vec2(239.182770f, 43.1293449f), vec2(234.989990f, 53.1800117f), vec2(234.989990f, 53.1800117f));

    // path.move_to(delta + tests[i][0]);
    // path.cubic_to(delta + tests[i][1], delta + tests[i][2], delta + tests[i][3]);

    // path.move_to(vec2(132.0, 586.0));
    // path.cubic_to(vec2(285.0, 630.0), vec2(446.0, 604.0), vec2(584.0, 538.0));

    graphick::editor::Entity test_entity = graphick::editor::Editor::scene().create_element(path);
    graphick::editor::FillComponent fill =
      test_entity.add_component<graphick::editor::FillComponent>(graphick::vec4{0.8f, 0.3f, 0.3f, 1.0f});
    graphick::editor::StrokeComponent stroke =
      test_entity.add_component<graphick::editor::StrokeComponent>(graphick::vec4{0.93f, 0.64f, 0.74f, 1.0f}, 50.0f);

    // const_cast<graphick::editor::StrokeComponent::Data*>(&stroke.stroke_TEMP())->width = 10.0f;
    // const_cast<graphick::editor::StrokeComponent::Data*>(&stroke.stroke_TEMP())->cap = graphick::geom::LineCap::Round;
    // const_cast<graphick::editor::StrokeComponent::Data*>(&stroke.stroke_TEMP())->join = graphick::geom::LineJoin::Round;

    break;
  }

  // path1.move_to({ 0.0f, 0.0f });
  // path1.cubic_to({ -2.0f, 10.0f }, { -2.0f, 90.0f }, { 0.0f, 100.0f });
  // path1.cubic_to({ 0.0f, 100.0f }, { 210.0f, -300.0f }, { 200.0f, 100.0f });
  // path1.line_to({ 230.0f, 70.0f });
  // path1.cubic_to({ 225.0f, 73.0f }, { 173.0f, 0.0f }, { 170.0f, -60.0f });
  // path1.cubic_to({ 80.0f, 0.0f }, { 500.0f, 70.0f }, { 0.0f, 0.0f });
  // path1.close();

  // path1.move_to({ 0.0f, 0.0f });
  // path1.cubic_to({ 0.0f, 0.0f }, { -47.0f, -36.0f }, { -50.0f, -40.0f });
  // path1.cubic_to({ -50.0f, -40.0f }, { -70.0f, -85.0f }, { -110.0f, -85.0f });
  // path1.cubic_to({ -110.0f, -85.0f }, { -170.0f, -85.0f }, { -220.0f, -45.0f });

  // path1.move_to({ 100.0f, 0.0f });
  // path1.line_to({ 20.0f, -20.0f });
  // path1.line_to({ 80.0f, 0.0f });
  // path1.line_to({ 200.0f, -50.0f });
  // path1.line_to({ 100.0f, -20.0f });
  // path1.line_to({ 350.0f, -50.0f });
  // path1.line_to({ 320.0f, -40.0f });
  // path1.line_to({ 360.0f, 260.0f });

  // path1.close();

  graphick::editor::Entity test_entity = graphick::editor::Editor::scene().create_element(path1);

  test_entity.add_component<graphick::editor::FillComponent>(graphick::vec4{0.8f, 0.3f, 0.3f, 1.0f});
  graphick::editor::StrokeComponent stroke =
    test_entity.add_component<graphick::editor::StrokeComponent>(graphick::vec4{0.93f, 0.64f, 0.74f, 1.0f});

  const_cast<graphick::editor::StrokeComponent::Data*>(&stroke.stroke_TEMP())->width = 20.0f;
  const_cast<graphick::editor::StrokeComponent::Data*>(&stroke.stroke_TEMP())->cap = graphick::renderer::LineCap::Round;
  const_cast<graphick::editor::StrokeComponent::Data*>(&stroke.stroke_TEMP())->join = graphick::renderer::LineJoin::Round;

  // test_entity1.add_component<graphick::editor::FillComponent>(graphick::vec4{ 1.0f, 0.3f, 0.3f, 1.0f });
  // test_entity1.add_component<graphick::editor::StrokeComponent>(graphick::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
#endif

  while (!glfwWindowShouldClose(window)) {
    OPTICK_FRAME("MainThread");
    GK_FRAME("MainThread");

    glfwPollEvents();

    const bool swap = graphick::editor::Editor::render_loop(glfwGetTime());

    if (swap) glfwSwapBuffers(window);
  }

  graphick::editor::Editor::shutdown();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
