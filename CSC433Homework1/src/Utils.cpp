#define _CRT_SECURE_NO_WARNINGS
#include "Includes.h"
#include <stdio.h>


/*
* loadImage loads a ppm P6 image. Currently does not support files with
* comments in them or P3 ppm images.
* 
* Takes in a const char* for the filepath which can be relative
* 
* Returns an Image object.
* 
* Requirements, only takes PPm P6 files. Does not check for incorrectly formatted files.
* Requires that each pixel is between 0 and 255, or 1 byte. 
*/
Image loadImage(const char* fileName) {

	char c;
	int type, width_, height_, maxSize;

	// Open the file
	FILE* file = fopen(fileName, "rb");

	// Get the type of PPM file
	fscanf(file, "%c%d", &c, &type);

	// Get the width and height
	fscanf(file, "%d %d", &width_, &height_);

	// And the max value of a pixel
	fscanf(file, "%d", &maxSize);

	// Log information to console
	printf("Type: %c%d\n", c, type);
	printf("W: %d, H: %d\n", width_, height_);
	printf("Max Size: %d\n", maxSize);

	// Create a buffer of Pixel objects for easier acessing later
	Pixel* pixels = (Pixel*)malloc(sizeof(Pixel) * width_ * height_);

	// copy the data from the file to this buffer
	uint8_t* buff = (uint8_t*)malloc(sizeof(uint8_t) * width_ * height_ * 3);
	fread(buff, sizeof(uint8_t), width_ * height_ * 3, file);


	// Now convert from uint8_t buffer to Pixel buffer and save
	int cntr = 0;
	for (int i = 0; i < width_*height_; i++) {
		// For whatever reason, we read brg rather than rgb.
		// Not sure why but the image looks correct
		pixels[i].b = (uint8_t)buff[cntr++];
		pixels[i].r = (uint8_t)buff[cntr++];
		pixels[i].g = (uint8_t)buff[cntr++];

		uint8_t v = buff[i];
	}

	Image img = { width_, height_, pixels };

	return img;
}



// just raw strings, very simple shaders so will leave as a long string
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec2 textCoord;\n"

"out vec2 TexCord;\n"
"void main()\n"
"{\n"
"   TexCord=textCoord;\n"
"   gl_Position = vec4(pos,1);\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCord;\n"
"uniform sampler2D texture1;\n"
"void main()\n"
"{\n"
"   FragColor = texture(texture1,TexCord);\n"
"}\n\0";



/*
* loadShaders loads the fragment and vertex shaders and creates the opengl program 
* object
*/
void loadShaders() {
	// Create shaders on the GPU
	unsigned int vertexShader;
	unsigned int fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load the source code from the strings and compile
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	// Create our program and attach/link shaders
	unsigned int program;
	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glUseProgram(program);

	// Delete the shaders since they are in the program now
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

/*
* genTexture generates a texture on the GPU and returns a heap buffer 
* The heap buffer is width x height x 3(rgb) of uint8_t 
*/
uint8_t* genTexture(int width, int height) {

	// Create and bind texture on the GPU
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// Allocate space for width x height pixels of RGB
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// And generate the Mipmap
	glGenerateMipmap(GL_TEXTURE_2D);

	// Create the heap buffer and return it.
	const int channels = 3;
	uint8_t* data = (uint8_t*)malloc(sizeof(uint8_t) * width * height * channels);

	return data;
}

/*
* setupTriangles sets up our VAO and VBO for rendering and sends the vertex
* data to the GPU. Just call this once 
*/
void setupTriangles() {
	// Create our Vertex Array Object
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create our Vertex Buffer Object
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	// This is our 2 triangles to draw on the GPU which span the whole screen
	//order: x,y,z,u,v
	float vertices[] = {
	-1,-1,0,0,0,
	-1,1,0,0,1,
	1,-1,0,1,0,
	-1,1,0,0,1,
	1,1,0,1,1,
	1,-1,0,1,0
	};

	// Send the data to the GPU and then process it
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// do position first
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	// texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
	
	// Enable the vertex arrays
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

}


/*
* Generic framebuffer callback to resize the window
*/
void framebuffer_size_callback(GLFWwindow* window, int width_, int height_) {
	glViewport(0, 0, width_, height_);
}


/*
* initRenderer creates the window which we draw to and setup anything else
* needed for rendering. Returns the GLFWwindow* object for reference later if needed.
*/
GLFWwindow* initRenderer(int winWidth, int winHeight) {

	// Init the window with GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(winWidth, winHeight, "Window", NULL, NULL);
	if (window == NULL) {
		std::cout << "failed to create\n";
		exit(1);
	}
	glfwMakeContextCurrent(window);

	// Load glad so we can get our opengl functions easier
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initalize GLAD\n");
		exit(1);
	}

	// Set resizing and the viewport
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glViewport(0, 0, winWidth, winHeight);

	// Load the shaders and program
	loadShaders();

	// Setup vbo for rendering the two triangles
	setupTriangles();
	

	return window;
}