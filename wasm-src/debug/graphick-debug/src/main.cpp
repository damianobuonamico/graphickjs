#include <stdio.h>
#include <iostream>

#include "../wasm-src/math/mat3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main() {
  GLFWwindow* window;

  if (!glfwInit()) {
    printf("Failed to initialize GLFW\n");
    return -1;
  }

  window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);

  mat3 vec{ };

  std::cout << vec << std::endl;

  if (!window) {
    printf("Failed to create window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  
  glfwTerminate();
}
