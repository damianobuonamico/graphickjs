#include <stdio.h>
#include <iostream>

#include "../wasm-src/editor/editor.h"
#include "../wasm-src/editor/input/input_manager.h"

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

  int width = 960;
  int height = 680;

  window = glfwCreateWindow(width, height, "Hello World", nullptr, nullptr);

  if (!window) {
    printf("Failed to create window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetWindowSizeCallback(window, window_resize_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetCursorEnterCallback(window, cursor_enter_callback);
  glfwSetKeyCallback(window, key_callback);

  Editor::init();
  InputManager::on_resize_event(width, height, 0, 0);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();

    Editor::render();
    glfwSwapBuffers(window);
  }

  Editor::shutdown();
  glfwTerminate();

  return 0;
}
