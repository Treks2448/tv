#include <GLFW/glfw3.h>
#include <iostream>

void error_callback(int error, const char* description) {
		fprintf(stderr, "Error: %s\n", description);
}

int main(){
	if (!glfwInit()) {
		return 1;
	}
	
	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(640, 480, "TV", NULL, NULL);
	if (!window) {
		return 1;
	}

	glfwMakeContextCurrent(window);
	
	while (!glfwWindowShouldClose(window)) {
		// keep running
		
	}
	if (glfwWindowShouldClose(window)) {
		glfwDestroyWindow(window);
	}
		
	
	glfwTerminate();
	return 0;
}
