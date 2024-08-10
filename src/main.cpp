#include <chrono>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <string>

#include "loadFrame.cpp"
#include "VideoStreamManager.h"

//int init(std::string filename, int& width, int& height, uint8_t** RGB_frame_buf, int& RGB_frame_buf_size);

void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %d: %s\n", error, description);
}

const char* vertexSource = R"glsl(
  #version 150 core
  in vec2 position;
  in vec3 color;
  in vec2 texcoord;

  out vec3 Color;
  out vec2 Texcoord;

  void main() {
    Color = color;
    Texcoord = texcoord;
    gl_Position = vec4(position, 0.0, 1.0);
  }
)glsl";

const char* fragmentSource = R"glsl(
  #version 150 core 
  in vec3 Color;
  in vec2 Texcoord;

  out vec4 outColor; 

  uniform sampler2D tex;
  void main() {
      outColor = texture(tex, Texcoord);
  }
)glsl";


int main() {
  int win_width = 500;
  int win_height = 300;
  int width = win_width;
  int height = win_height;

  std::string filename = "/home/igor/Programming/tv/q.mp4"; 
  VideoStreamManager::ManagerState managerState;
  VideoStreamManager videoStreamManager{filename, width, height};

  if (!glfwInit()) {
    return 1;
  }

  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  GLFWwindow *window = glfwCreateWindow(win_width, win_height, "TV", nullptr, nullptr);
  //GLFWwindow *window = glfwCreateWindow(640, 480, "TV", glfwGetPrimaryMonitor(), nullptr); // fullscreen 
  if (!window) {
    return 1;
  }
  glfwMakeContextCurrent(window); 

  glewInit();
  
  // Init Vertex Array Object
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);



  // Shaders
  GLfloat vertices[] = {
  //  Position      Color             Texcoords
      -1.f,  1.f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
       1.f,  1.f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
       1.f, -1.f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
      -1.f, -1.f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
  };

  // Create vertex buffer object and bind (set active)
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // Copy vertex array vertices to buffer 
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 
  // Create an element array
  GLuint ebo;
  glGenBuffers(1, &ebo);

  GLuint elements[] = {
      0, 1, 2,
      2, 3, 0
  };

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

  // Vertex shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);

  GLint status;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    std::cout << "Failed to compile vertexShader. Status: " << status << "\n";
  }
  else {
    std::cout << "Compiled correctly" << "\n";
  }
  
  // Fragment shader
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    std::cout << "Failed to compile fragmentShader. Status: " << status << "\n";
  }
  else {
    std::cout << "Compiled correctly" << "\n";
  }

  // Create program from shaders
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);

  //glBindFragDataLocation(shaderProgram, 0, "outColor");
 
  // Link and use program
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);
 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
  
  // Specify how the input position attribute (of the vertex shader) is structured 
  // and enable the attribute position
  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
  glEnableVertexAttribArray(posAttrib);

  GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
  glEnableVertexAttribArray(colAttrib);
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));

  GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  
  auto t_start = std::chrono::high_resolution_clock::now();
  
  uint8_t* pixels = nullptr;
  while (VideoStreamManager::ManagerState::GOT_VIDEO_PKT != videoStreamManager.processNextPacket());
  pixels = videoStreamManager.getRGBBuffer();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  auto t_now = std::chrono::high_resolution_clock::now();
  float time_since_last_frame = 0;
  while (!glfwWindowShouldClose(window)) {
    auto t_begin = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
    
    // Set triangle color
    GLint uniColor = glGetUniformLocation(shaderProgram, "triangleColor");
    glUniform3f(uniColor, (sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);
    if (time_since_last_frame > 0.01666) {
      time_since_last_frame = 0;
      //std::cout<<(static_cast<unsigned int>(time));
      managerState = videoStreamManager.processNextPacket();
      if (managerState == VideoStreamManager::ManagerState::GOT_VIDEO_PKT) {
        pixels = videoStreamManager.getRGBBuffer();
        //for (int i = 0; i < width*height*3; i+=3) {
        //  printf("{%u, %u, %u}", (int)pixels[i], (int)pixels[i+1], (int)pixels[i+2]);
        //  if (i % (3*width) == 0) {
        //    std::cout << "\n";
        //  }
        //}
        //std::cout << "Next frame ***** \n\n";
        // update what is drawn
      }
    }
    
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
    glfwPollEvents(); 
    auto t_end = std::chrono::high_resolution_clock::now();
    float time_delta = std::chrono::duration_cast<std::chrono::duration<float>>(t_end - t_begin).count();
    time_since_last_frame += time_delta;
  }
    
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
