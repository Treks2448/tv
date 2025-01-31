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
  #version 330 core 
  in vec3 Color;
  in vec2 Texcoord;

  layout(location = 0) out vec4 outColor; 

  uniform sampler2D tex;
  void main() {
      outColor = texture(tex, Texcoord);
  }
)glsl";

GLuint initShaderProgram() {
  // Vertex shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);

  GLint status;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    std::cout << "Failed to compile vertexShader. Status: " << status << "\n";
    return 1;
  }
  
  // Fragment shader
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    std::cout << "Failed to compile fragmentShader. Status: " << status << "\n";
    return 1;
  }

  // Create program from shaders
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  
  // This line of code might be necessary when using a custom fbo
  // glBindFragDataLocation(shaderProgram, fbo? , "outColor");
 
  // Link and use program
  glLinkProgram(shaderProgram);

  return shaderProgram;
}

int main() {
  int win_width = 1920;
  int win_height = 1080;
  int width = win_width;
  int height = win_height;

  std::string filename = "/home/igor/Programming/tv/x.mkv"; 
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
 
  // The currently bound vertex array object store references to vbos and attributes (see below).
  // This is primarily for organisational purposes when multiple shaders and vertex layouts are
  // used in a program. We probably won't need this.
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // These vertices form the rectangle over which the video is displayed
  GLfloat vertices[] = {
  //  Position     Color values       Texture coords
      -1.f,  1.f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
       1.f,  1.f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
       1.f, -1.f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
      -1.f, -1.f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f
  };
  // Create vertex buffer object, bind it and copy vertex data to it
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // NOTE: GL_STATIC_DRAW may be inefficient - consider alternatives
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 
  // Element buffer objects allow us to conrol the order in which vertices are rendered.
  // This can reduce the number of vertices that need to be rendered by reusing vertices.
  // Probably negligible performance impact on this application.
  // An element buffer is drawn using glDrawElements (see main loop)
  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  GLuint elements[] = {
      0, 1, 2,
      2, 3, 0
  };
  // NOTE: GL_STATIC_DRAW may be inefficient - consider alternatives
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

  GLuint shaderProgram = initShaderProgram();
  glUseProgram(shaderProgram);

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

  // Create texture, bind it and set formatting options
  GLuint tex;
  glGenTextures(1, &tex);
  glActiveTexture(tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // NOTE: Not sure if this is actually used correctly
  glGenerateMipmap(GL_TEXTURE_2D);

  // Get first frame of video as pixel array and attach to texture
  uint8_t* pixels = nullptr;
  while (VideoStreamManager::ManagerState::GOT_VIDEO_PKT != videoStreamManager.processNextPacket());
  pixels = videoStreamManager.getRGBBuffer();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
  
  // Create two pixel buffers for asynchronouse DMA transfer
  const int n_pbos = 2;
  int pbo_index = 0;
  GLuint pbos[n_pbos];
  glGenBuffers(n_pbos, pbos);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[0]);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * 3, 0, GL_STREAM_DRAW);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[1]);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * 3, 0, GL_STREAM_DRAW);
  
  float time_since_last_frame = 0;
  while (!glfwWindowShouldClose(window)) {
    // Take time measurement at the start of the main loop
    auto t_begin = std::chrono::high_resolution_clock::now();
    
    //if (time_since_last_frame > 0.033333) {
      time_since_last_frame = 0;
      managerState = videoStreamManager.processNextPacket();
 
      if (managerState == VideoStreamManager::ManagerState::GOT_VIDEO_PKT)
      {
        glBindTexture(GL_TEXTURE_2D, tex);
        
        // copy pixels from PBO to texture object
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[pbo_index % n_pbos]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, 0);
        
        // update pixels and copy to PBO
        pixels = videoStreamManager.getRGBBuffer();
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[(pbo_index + 1) % n_pbos]);
        GLubyte* ptr = reinterpret_cast<GLubyte*>(
          glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY)
        );
        if (ptr)
        {
          memcpy(ptr, pixels, static_cast<size_t>(width * height * 3));
          glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }
        pbo_index = (pbo_index + 1) % n_pbos;
      }
    //}

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    glfwSwapBuffers(window);
    glfwPollEvents(); 

    // Take time measurement at the end of the main loop
    auto t_end = std::chrono::high_resolution_clock::now();

    // Update frame data
    float time_delta = std::chrono::duration_cast<std::chrono::duration<float>>(t_end - t_begin).count();
    time_since_last_frame += time_delta;
  }

  // TODO: probably need to delete a bunch of other OpenGL objects here
  //glDeleteFramebuffers(1, &fbo); 
 
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
