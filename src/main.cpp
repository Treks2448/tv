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
  out vec3 Color;
  void main() {
    Color = color;
    gl_Position = vec4(position, 0.0, 1.0);
  }
)glsl";

const char* fragmentSource = R"glsl(
  #version 150 core 
  in vec3 Color;
  out vec4 outColor; 
  void main() {
      outColor = vec4(Color, 1.0);
  }
)glsl";


int main() {
  //std::string filename = "/home/igor/media/asdf/1.mp4"; 
  //VideoStreamManager::ManagerState managerState;
  //VideoStreamManager videoStreamManager{filename, 1920, 1032};

  if (!glfwInit()) {
    return 1;
  }

  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  GLFWwindow *window = glfwCreateWindow(640, 480, "TV", nullptr, nullptr);
  //GLFWwindow *window = glfwCreateWindow(640, 480, "TV", glfwGetPrimaryMonitor(), nullptr); // fullscreen 
  if (!window) {
    return 1;
  }
  glfwMakeContextCurrent(window); 

  glewInit();
  
  // Shaders
  float vertices[] = {
     0.0f,  0.5f, 1.0, 0.0, 0.0f, // Vertex 1 (X, Y) 
     0.5f, -0.5f, 0.0, 1.0, 0.0f, // Vertex 2 (X, Y)
    -0.5f, -0.5f, 0.0, 0.0, 1.0f  // Vertex 3 (X, Y)
  };

  // Create vertex buffer object and bind (set active)
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  
  // Copy vertex array vertices to buffer 
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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
 
  // 
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  
  // Specify how the input position attribute (of the vertex shader) is structured 
  // and enable the attribute position
  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
  glEnableVertexAttribArray(posAttrib);

  GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
  glEnableVertexAttribArray(colAttrib);
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

  auto t_start = std::chrono::high_resolution_clock::now();

  while (!glfwWindowShouldClose(window)) {
    auto t_now = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
    
    // Set triangle color
    GLint uniColor = glGetUniformLocation(shaderProgram, "triangleColor");
    glUniform3f(uniColor, (sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);

    //managerState = videoStreamManager.processNextPacket();
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
    glfwPollEvents(); 
  }

    
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}


  //// Shaders
  //GLint status;
  //constexpr int maxLogSize = 512;
  //char buffer[maxLogSize];
  //
  //GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  //glShaderSource(vertexShader, 1, &vertexSource, NULL);
  //glCompileShader(vertexShader);
  //glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status); 
  //if (status != GL_TRUE) {
  //  glGetShaderInfoLog(vertexShader, maxLogSize, NULL, buffer);
  //  std::cout << buffer;
  //}

  //GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  //glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  //glCompileShader(fragmentShader);
  //glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
  //if (status != GL_TRUE) {
  //  glGetShaderInfoLog(fragmentShader, maxLogSize, NULL, buffer);
  //  std::cout << buffer;
  //}

  //GLuint shaderProgram = glCreateProgram();
  //glAttachShader(shaderProgram, vertexShader);
  //glAttachShader(shaderProgram, fragmentShader);
  //glLinkProgram(shaderProgram);
  //glUseProgram(shaderProgram);

  //GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  //glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
  //glEnableVertexAttribArray(posAttrib);


  //GLuint vtxArrObj;
  //glGenVertexArrays(1, &vtxArrObj);
  //glBindVertexArray(vtxArrObj);



  //// temporary for testing
  //float vertices[] = {
  //  0.0f,  0.5f, GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  //glShaderSource(vertexShader, 1, &vertexSource, NULL);// Vertex 1 (X, Y)
  //  0.5f, -0.5f, // Vertex 2 (X, Y)
  //  -0.5f, -0.5f  // Vertex 3 (X, Y)
  //};

  //GLuint vtxObjBuf;
  //glGenBuffers(1, &vtxObjBuf);
  //glBindBuffer(GL_ARRAY_BUFFER, vtxObjBuf);
  //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);



  //GLuint tex;

  //glGenTextures(1, &tex);
  //glBindTexture(GL_TEXTURE_2D, tex);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  //float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
  //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, videoStreamManager.getRGBBuffer());
 
  //glBindTexture(GL_TEXTURE_2D, 0);
  
  //GLfloat vertices[] = {
  ////  Position      Color             Texcoords
  //    -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
  //     0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
  //     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
  //    -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
  //};
  //
  //GLuint elements[] = {
  //    0, 1, 2,
  //    2, 3, 0
  //};
  
  // Set up buffer objects, create shaders, initialize GL, etc.
  
  //drawing
  //bind buffers, enable attrib arrays, etc
  //glBindTexture(GL_TEXTURE_2D, tex);
