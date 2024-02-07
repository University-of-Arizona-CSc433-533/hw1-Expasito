#pragma once


#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>


/*
* Pixel struct which represents the r,g,b values of a pixel
*/
typedef struct Pixel {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} Pixel;


/*
* Image struct holds the Pixel data and the width and height of the image
*/
typedef struct Image {
	int width;
	int height;
	struct Pixel* data;
} Image;

