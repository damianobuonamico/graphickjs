#include "wasm-src/editor/editor.h"
#include "wasm-src/editor/scene/entity.h"

#include "wasm-src/geom/path.h"

#include "wasm-src/io/resource_manager.h"
#include "wasm-src/io/svg/svg.h"

#include "wasm-src/utils/debugger.h"

#include "callbacks.h"

#include <fstream>

using namespace graphick;

int main()
{
  GLFWwindow* window = create_glfw_window(800, 600);

  if (!window) {
    printf("Failed to create window\n");
    return -1;
  }

#define TEXT
#define IMAGES
// #define TIGER
#define OBJECTS

#ifdef TEXT
  std::ifstream font_file1("res/fonts/consolas.ttf", std::ios::binary | std::ios::ate);
  std::ifstream font_file2("res/fonts/times.ttf", std::ios::binary | std::ios::ate);

  std::streamsize font_size1 = font_file1.tellg();
  std::streamsize font_size2 = font_file2.tellg();

  std::vector<uint8_t> font_data1(font_size1);
  std::vector<uint8_t> font_data2(font_size2);

  font_file1.seekg(0, std::ios::beg);
  font_file2.seekg(0, std::ios::beg);

  font_file1.read(reinterpret_cast<char*>(font_data1.data()), font_size1);
  font_file2.read(reinterpret_cast<char*>(font_data2.data()), font_size2);

  utils::uuid font_id1 = io::ResourceManager::load_default_font(font_data1.data(),
                                                                font_data1.size());
  utils::uuid font_id2 = io::ResourceManager::load_font(font_data2.data(), font_data2.size());

  editor::Entity text1 = editor::Editor::scene().create_text("Hello, World!", font_id2);
  // editor::Editor::scene().create_text(
  //     "abcdefghijklmnopqrstuvwxyz1234567890|!\"£$%&/()=?^'ìèé[]*+ù§à°#@çò-_.:,;<>", font_id2);

  text1.add_component<editor::FillComponent>(vec4{0.8f, 0.3f, 0.3f, 1.0f});
#endif

#ifdef IMAGES
  std::ifstream image_file1("res/images/demo_image1.png", std::ios::binary | std::ios::ate);
  std::ifstream image_file2("res/images/demo_image3.png", std::ios::binary | std::ios::ate);

  std::streamsize image_size1 = image_file1.tellg();
  std::streamsize image_size2 = image_file2.tellg();

  std::vector<uint8_t> image_data1(image_size1);
  std::vector<uint8_t> image_data2(image_size2);

  image_file1.seekg(0, std::ios::beg);
  image_file2.seekg(0, std::ios::beg);

  image_file1.read(reinterpret_cast<char*>(image_data1.data()), image_size1);
  image_file2.read(reinterpret_cast<char*>(image_data2.data()), image_size2);

  utils::uuid image_id1 = io::ResourceManager::load_image(image_data1.data(), image_data1.size());
  utils::uuid image_id2 = io::ResourceManager::load_image(image_data2.data(), image_data2.size());

  editor::Editor::scene().create_image(image_id1);
  editor::Editor::scene().create_image(image_id2);
#endif

#ifdef TIGER
  std::ifstream ifs("res/vectors/Ghostscript_Tiger.svg");
  std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  io::svg::parse_svg(content);
#endif

#ifdef OBJECTS
  geom::path path1;

  path1.move_to(vec2(0.0f, 0.0f));

  path1.cubic_to(vec2(0.0f, 32.0f), vec2(16.0f, 128.0f), vec2(32.0f, 128.0f));
  path1.cubic_to(vec2(48.0f, 128.0f), vec2(64.0f, 32.0f), vec2(64.0f, 0.0f));
  // path1.line_to(vec2(16.0f, 128.0f));
  // path1.line_to(vec2(128.0f, -32.0f));

  // path1.cubic_to(vec2(0.0f, 100.0f), vec2(100.0f, 100.0f), vec2(100.0f, 0.0f));
  // path1.cubic_to(vec2(100.0f, -100.0f), vec2(0.0f, -100.0f), vec2(0.0f, 0.0f));

  // path1.line_to(vec2(100.0f, 0.0f));
  // path1.line_to(vec2(0.0f, 200.0f));

  path1.close();

  editor::Entity entity1 = editor::Editor::scene().create_element(path1);

  entity1.add_component<editor::FillComponent>(vec4{0.8f, 0.3f, 0.3f, 1.0f});
  entity1.add_component<editor::StrokeComponent>(vec4{0.93f, 0.64f, 0.74f, 1.0f}, 50.0f);

  geom::path path2;

  path2.move_to(vec2(-50.0f, 0.0f));

  path2.line_to(vec2(150.0f, -100.0f));
  path2.line_to(vec2(150.0f, 100.0f));

  path2.close();

  editor::Entity entity2 = editor::Editor::scene().create_element(path2);

  entity2.add_component<editor::FillComponent>(vec4{0.3f, 0.8f, 0.3f, 1.0f});
  entity2.add_component<editor::StrokeComponent>(vec4{0.64f, 0.93f, 0.74f, 1.0f}, 50.0f);

  geom::path path3;

  path3.move_to(vec2(0.0f, 0.0f));

  path3.line_to(vec2(100.0f, 0.0f));
  path3.line_to(vec2(100.0f, 100.0f));
  path3.line_to(vec2(0.0f, 100.0f));

  path3.close();

  editor::Entity entity3 = editor::Editor::scene().create_element(path3);

  entity3.add_component<editor::FillComponent>(vec4{0.3f, 0.3f, 0.8f, 1.0f});
  entity3.add_component<editor::StrokeComponent>(vec4{0.64f, 0.74f, 0.93f, 1.0f}, 50.0f);
#endif

  while (!glfwWindowShouldClose(window)) {
    __debug_time_frame();

    glfwPollEvents();

#ifdef GK_DEBUG
    graphick::editor::Editor::request_render(false);
#endif

    const bool swap = graphick::editor::Editor::render_loop(glfwGetTime());

    if (swap) {
      glfwSwapBuffers(window);
    }
  }

  graphick::editor::Editor::shutdown();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
