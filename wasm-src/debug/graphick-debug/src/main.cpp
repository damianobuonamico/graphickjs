#include <stdio.h>
#include <iostream>

#include "wasm-src/editor/editor.h"
#include "wasm-src/editor/input/input_manager.h"
#include "wasm-src/renderer/geometry/corners_detection.h"
#include "wasm-src/renderer/geometry/stroker.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct PointerState {
  vec2 position;
  InputManager::PointerButton button;
  bool alt;
  bool ctrl;
  bool shift;
};

static PointerState pointer_state;

static void cursor_position_callback(GLFWwindow* window, double x, double y) {
  pointer_state.position = { (float)x, (float)y };

  InputManager::on_pointer_event(
    InputManager::PointerTarget::Canvas, InputManager::PointerEvent::Move,
    InputManager::PointerType::Mouse, pointer_state.button,
    pointer_state.position.x, pointer_state.position.y, 1.0f, glfwGetTime() * 1000,
    pointer_state.alt, pointer_state.ctrl, pointer_state.shift
  );
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  pointer_state.button = (InputManager::PointerButton)button;

  InputManager::on_pointer_event(
    InputManager::PointerTarget::Canvas, action == GLFW_PRESS ? InputManager::PointerEvent::Down : InputManager::PointerEvent::Up,
    InputManager::PointerType::Mouse, pointer_state.button,
    pointer_state.position.x, pointer_state.position.y, 1.0f, glfwGetTime() * 1000,
    pointer_state.alt, pointer_state.ctrl, pointer_state.shift
  );
}

static void window_resize_callback(GLFWwindow* window, int width, int height) {
  InputManager::on_resize_event(width, height, 0, 0);
}

static void scroll_callback(GLFWwindow* window, double delta_x, double delta_y) {
  InputManager::on_wheel_event(InputManager::PointerTarget::Canvas, (float)delta_x * 100.0f, -(float)delta_y * 100.0f);
}

static void cursor_enter_callback(GLFWwindow* window, int entered) {
  InputManager::on_pointer_event(
    InputManager::PointerTarget::Canvas, entered ? InputManager::PointerEvent::Enter : InputManager::PointerEvent::Leave,
    InputManager::PointerType::Mouse, pointer_state.button,
    pointer_state.position.x, pointer_state.position.y, 1.0f, glfwGetTime() * 1000,
    pointer_state.alt, pointer_state.ctrl, pointer_state.shift
  );
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers) {
  pointer_state.alt = modifiers & GLFW_MOD_ALT;
  pointer_state.ctrl = modifiers & GLFW_MOD_CONTROL;
  pointer_state.shift = modifiers & GLFW_MOD_SHIFT;

  InputManager::on_keyboard_event(
    action == GLFW_RELEASE ? InputManager::KeyboardEvent::Up : InputManager::KeyboardEvent::Down, (KeyboardKey)key, action == GLFW_REPEAT,
    pointer_state.alt, pointer_state.ctrl, pointer_state.shift
  );
}

int main() {
  GLFWwindow* window;

  if (!glfwInit()) {
    printf("Failed to initialize GLFW\n");
    return -1;
  }

  int width = 1300;
  int height = 810;
  int samples = 8;

  glfwWindowHint(GLFW_SAMPLES, samples);

  window = glfwCreateWindow(width, height, "Hello World", nullptr, nullptr);

  if (!window) {
    printf("Failed to create window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
  //io.ConfigViewportsNoAutoMerge = true;
  //io.ConfigViewportsNoTaskBarIcon = true;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetWindowSizeCallback(window, window_resize_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetCursorEnterCallback(window, cursor_enter_callback);
  glfwSetKeyCallback(window, key_callback);

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330 core");

  Editor::init();
  InputManager::on_resize_event(width, height, 0, 0);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Debug Settings");                          // Create a window called "Debug Settings" and append into it.

      ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

      ImGui::SliderFloat("min_radius", &min_radius, 0.0f, 100.0f);
      ImGui::SliderFloat("max_radius", &max_radius, 0.0f, 100.0f);
      ImGui::SliderInt("max_iterations", (int*)&max_iterations, 0, 100);
      ImGui::SliderAngle("min_angle", &min_angle, 0.0f, 360.0f);
      ImGui::Checkbox("simplify_first", &simplify_first);
      ImGui::SliderFloat("simplification_tolerance", &simplification_tolerance, 0.0f, 10.0f);
      ImGui::SliderFloat("max_fit_error", &max_fit_error, 0.0f, 5.0f);
      ImGui::SliderAngle("max_angle", &max_angle, 0.0f, 20.0f);
      ImGui::SliderFloat("stroke_width", &stroke_width, 0.01f, 50.0f);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    Editor::render(true);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }
    glfwSwapBuffers(window);
  }

  Editor::shutdown();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
