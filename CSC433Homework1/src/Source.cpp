#define _CRT_SECURE_NO_WARNINGS
#include "Includes.h"

// Function definitions from Utils.cpp
GLFWwindow* initRenderer(int width, int height);
uint8_t* genTexture(int width, int height);
Image loadImage(const char* fileName);


// Width and height of the display/texture to draw to
int width = 1000;
int height = 850;


int main() {

	// Load our PPM P6 image
	Image img = loadImage("Assets/pic1.ppm");

	// Make the window's aspect ratio match the image being imported
	float aspect = img.width / (float)img.height;
	height = width / aspect;


	// Create the window and also initalize anything for rendering
	GLFWwindow* window = initRenderer(width, height);

	// Create the buffer for writing pixels and the gl texture
	uint8_t* data = genTexture(width, height);

	// stuff for measuring performance
	std::chrono::steady_clock::time_point begin;
	std::chrono::steady_clock::time_point end;
	float milis = 0;


	// Record the value of theta for the rotations
	float theta = 0.0f;


	// The per-frame loop
	while (!glfwWindowShouldClose(window)) {

		// Start measuring performance
		begin = std::chrono::steady_clock::now();

		// Rotate clockwise so subtract values
		theta -= .1;


		// These are all matricies used for transforming coordinates from Target to Source
		// GLM has a funny way of creating matricies so the rows and cols are flipped, which is why
		// The transform matricies look backwards


		// Convert from 0, (width-1) to 0,(width) which works better for scaling to -1,1
		glm::mat3 convertTo = {
			glm::vec3(((float)width) / (width - 1), 0, 0),
			glm::vec3(0, ((float)height) / (height - 1), 0),
			glm::vec3(0,0, 1)
		};

		// Shift center of target image to 0,0
		glm::mat3 beforeTrans = {
			glm::vec3(1.0, 0.0, 0),
			glm::vec3(0.0, 1.0, 0),
			glm::vec3(-width / 2.0, -height / 2.0, 1.0)
		};

		// Scale target image's range to -1,1
		glm::mat3 scale = {
			glm::vec3(2.0f / (width), 0, 0),
			glm::vec3(0, 2.0f / (height), 0),
			glm::vec3(0,0, 1)
		};

		// Scale up to source image's width and height
		glm::mat3 scale_ = {
			glm::vec3((float)img.width / 2.0f, 0, 0),
			glm::vec3(0, (float)img.height / 2.0f, 0),
			glm::vec3(0,0, 1)
		};

		// Reflect over y axis
		glm::mat3 Reflect = {
			glm::vec3(1, 0, 0),
			glm::vec3(0, -1, 0),
			glm::vec3(0, 0, 1)
		};

		// Rotate image by theta value
		glm::mat3 Rotate = {
			glm::vec3(cos(theta), sin(theta), 0),
			glm::vec3(-sin(theta), cos(theta), 0),
			glm::vec3(0, 0, 1)
		};
		
		// Multiply by 2 (because we are inverting the matricies) to scale the image down by 2 to be fully visible on screen
		glm::mat3 scale2 = {
			glm::vec3(2.0f, 0, 0),
			glm::vec3(0, 2.0f, 0),
			glm::vec3(0,0, 1)
		};

		// Move image from 0,0 to old center, but for the source image
		glm::mat3 afterTrans = {
			glm::vec3(1, 0, 0),
			glm::vec3(0, 1, 0),
			glm::vec3(img.width / 2.0, img.height / 2.0, 1)
		};

		// Convert from 0, width to 0,(width-1) which are the index coordinates
		glm::mat3 convertBack = {
			glm::vec3((img.width - 1) / (float)(img.width), 0, 0),
			glm::vec3(0, (img.height - 1) / (float)(img.height), 0),
			glm::vec3(0,0, 1)
		};

		

		// There are more steps than required, but the logic behind each transformation is present
		// Multiply all matricies so we can have one matrix multiplication per pixel
		glm::mat3 transform = 
			convertBack * 
			afterTrans * 
			scale2 * 
			Rotate * 
			Reflect * 
			scale_ * 
			scale * 
			beforeTrans *
			convertTo;



		// Iterate over every pixel in the output image
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {

				// get index of current pixel for the screen. We mult by 3 to account for the rgb chars in the image
				int index =  3 * i * width + j * 3;

				// homogeneous coordinates
				glm::vec3 pt = { j, i, 1};


				// Use our transform matrix to get the new coordinates in the source image
				glm::vec3 pt2 = transform * pt;

				// Round to an integer value
				pt2.x = (int)round(pt2.x);
				pt2.y = (int)round(pt2.y);


				// Check if the pixel location is within the bounds of the image
				if (pt2.y < 0 || pt2.y >= img.height || pt2.x < 0 || pt2.x >= img.width) {
					// Out of bounds so set to black background
					data[index] = 0;
					data[index + 1] = 0;
					data[index + 2] = 0;

				}
				else {
					// Get the index and pixel. Then copy
					int index2 = img.width * pt2.y + pt2.x;
					Pixel p = img.data[index2];
					
					data[index] = p.r;
					data[index + 1] = p.g;
					data[index + 2] = p.b;


				}
			}
		}

		// Stop clocking performance
		end = std::chrono::steady_clock::now();

		// update image for opengl to draw. Texture is already bound so no need to redo that
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

		// draw image
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(window);
		glfwPollEvents();


		// get delta time and frame data
		milis = (end - begin).count() / 1000000.0;
		std::cout << "Time difference = " << milis << "[ms]" << " FPS: " << 1000.0 / milis << "\n";

	}


}