#include <stdio.h>
#include <iostream>
#include <fstream>
#include <direct.h>

#include "wasm-src/editor/editor.h"
#include "wasm-src/editor/text/font_manager.h"
#include "wasm-src/editor/settings.h"
#include "wasm-src/editor/input/input_manager.h"

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
  InputManager::on_resize_event(width, height, 1.0f, 0, 0);
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
  InputManager::on_resize_event(width, height, 1.0f, 0, 0);

  Editor::load("{\"version\":\"0.1.0\",\"files\":[{\"head\":{\"id\":\"12302782324244206606\",\"viewport\":{\"position\":[0,0],\"zoom\":2,\"rotation\":0,\"min_position\":[-3.40282e+38,-3.40282e+38],\"max_position\":[3.40282e+38,3.40282e+38],\"min_zoom\":0.01}},\"body\":{}}]}");

  const char* pathname = "res\\GreatVibes-Regular.otf";
  FILE* file = fopen(pathname, "rb");

  if (!file) {
    console::error("Failed to open font file", pathname);
  } else {
    fseek(file, 0, SEEK_END);
    unsigned long size = ftell(file);
    if (!size) {
      console::error("Failed to get font file size", pathname);
      fclose(file);
    }

    fseek(file, 0, SEEK_SET);

    unsigned char* buffer = new unsigned char[size];
    fread(buffer, 1, size, file);


    FontManager::load_font(buffer, size);
  }

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

      ImGui::SliderFloat("Mass", &Settings::mass_constant, 0.001f, 1.0f);
      ImGui::SliderFloat("Spring", &Settings::spring_constant, 0.01f, 10.0f);
      ImGui::SliderFloat("Viscosity", &Settings::viscosity_constant, 0.01f, 1.8f);
      ImGui::SliderFloat("Corners Min Radius", &Settings::corners_radius_min, 0.001f, 5.0f);
      ImGui::SliderFloat("Corners Max Radius", &Settings::corners_radius_max, 0.0002f, 20.0f);
      ImGui::SliderInt("Corners Max Samples", (int*)&Settings::corners_samples_max, 0, 20);
      ImGui::SliderAngle("Corners Angle Threshold", &Settings::corners_angle_threshold, 0.1f, 90.0f);
      ImGui::SliderFloat("Corners Min Distance", &Settings::corners_min_distance, 0.001f, 100.0f);
      ImGui::SliderFloat("Simplification Threshold", &Settings::simplification_threshold, 0.0f, 10.0f);
      ImGui::Checkbox("Upsample Before Fitting", &Settings::upsample_before_fitting);
      ImGui::SliderFloat("Fit Max Error", &Settings::max_fit_error, 0.01f, 5.0f);
      ImGui::SliderAngle("Facet Angle", &Settings::facet_angle, 0.0f, 20.0f);
      ImGui::SliderFloat("Tessellation Error", &Settings::tessellation_error, 0.01f, 1.0f);

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
