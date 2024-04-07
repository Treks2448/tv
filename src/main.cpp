#include <GLFW/glfw3.h>
#include <iostream>

#include "loadFrame.cpp"

int init(std::string filename, int& width, int& height, uint8_t** RGB_frame_buf, int& RGB_frame_buf_size);

void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %d: %s\n", error, description);
}

int main() {
  int width;
  int height;
  uint8_t* RGB_pix_buf = nullptr;
  int RGB_pix_buf_size = 0;
  init("/home/igor/media/JuOn-The-Grudge (2002)/JuOn-The-Grudge.mp4", width, height, &RGB_pix_buf, RGB_pix_buf_size); 

  if (!glfwInit()) {
    return 1;
  }

  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window = glfwCreateWindow(640, 480, "TV", NULL, NULL);
  if (!window) {
    return 1;
  }

  glfwMakeContextCurrent(window);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents(); 
  }

    
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
