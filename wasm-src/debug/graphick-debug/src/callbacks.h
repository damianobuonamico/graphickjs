#pragma once

#include "wasm-src/editor/input/input_manager.h"

#include "wasm-src/math/math.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

struct PointerState {
  graphick::vec2 position;
  graphick::editor::input::InputManager::PointerButton button;
  bool alt;
  bool ctrl;
  bool shift;
};

static PointerState pointer_state;
static float dpr;

static void cursor_position_callback(GLFWwindow* window, double x, double y)
{
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
      pointer_state.shift);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  pointer_state.button = (graphick::editor::input::InputManager::PointerButton)button;

  graphick::editor::input::InputManager::on_pointer_event(
      graphick::editor::input::InputManager::PointerTarget::Canvas,
      action == GLFW_PRESS ? graphick::editor::input::InputManager::PointerEvent::Down :
                             graphick::editor::input::InputManager::PointerEvent::Up,
      graphick::editor::input::InputManager::PointerType::Mouse,
      pointer_state.button,
      pointer_state.position.x,
      pointer_state.position.y,
      1.0f,
      glfwGetTime() * 1000,
      pointer_state.alt,
      pointer_state.ctrl,
      pointer_state.shift);
}

static void window_resize_callback(GLFWwindow* window, int width, int height)
{
  graphick::editor::input::InputManager::on_resize_event(
      (int)(width / dpr), (int)(height / dpr), dpr, 0, 0);
}

static void scroll_callback(GLFWwindow* window, double delta_x, double delta_y)
{
  graphick::editor::input::InputManager::on_wheel_event(
      graphick::editor::input::InputManager::PointerTarget::Canvas,
      -(float)delta_x * 0.75f,
      -(float)delta_y * 0.75f,
      pointer_state.ctrl);
}

static void cursor_enter_callback(GLFWwindow* window, int entered)
{
  graphick::editor::input::InputManager::on_pointer_event(
      graphick::editor::input::InputManager::PointerTarget::Canvas,
      entered ? graphick::editor::input::InputManager::PointerEvent::Enter :
                graphick::editor::input::InputManager::PointerEvent::Leave,
      graphick::editor::input::InputManager::PointerType::Mouse,
      pointer_state.button,
      pointer_state.position.x,
      pointer_state.position.y,
      1.0f,
      glfwGetTime() * 1000,
      pointer_state.alt,
      pointer_state.ctrl,
      pointer_state.shift);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
  pointer_state.alt = modifiers & GLFW_MOD_ALT;
  pointer_state.ctrl = modifiers & GLFW_MOD_CONTROL;
  pointer_state.shift = modifiers & GLFW_MOD_SHIFT;

  graphick::editor::input::InputManager::on_keyboard_event(
      action == GLFW_RELEASE ? graphick::editor::input::InputManager::KeyboardEvent::Up :
                               graphick::editor::input::InputManager::KeyboardEvent::Down,
      (graphick::editor::input::KeyboardKey)key,
      action == GLFW_REPEAT,
      pointer_state.alt,
      pointer_state.ctrl,
      pointer_state.shift);
}

static GLFWwindow* create_glfw_window(const int width, const int height)
{
  GLFWwindow* window;

  if (!glfwInit()) {
    printf("Failed to initialize GLFW\n");
    return nullptr;
  }

  window = glfwCreateWindow(width, height, "graphick", nullptr, nullptr);
  glfwSetWindowPos(window, 0, 30);

  if (!window) {
    glfwTerminate();
    return nullptr;
  }

  float x_scale, y_scale;
  glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &x_scale, &y_scale);

  dpr = (x_scale + y_scale) / 2.0f;

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync
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
  graphick::editor::input::InputManager::on_resize_event(
      (int)(width / dpr), (int)(height / dpr), dpr, 0, 0);

  return window;
}
