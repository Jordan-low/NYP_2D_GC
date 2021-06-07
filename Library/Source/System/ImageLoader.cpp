/**
 CImageLoader
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "ImageLoader.h"

#include <iostream>
using namespace std;

// Include GLEW
#include <GL/glew.h>

// STB Image
#ifndef STB_IMAGE_IMPLEMENTATION
	#define STB_IMAGE_IMPLEMENTATION
#endif
#include <includes/stb_image.h>
#include "filesystem.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CImageLoader::CImageLoader(void)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CImageLoader::~CImageLoader(void)
{
}

/**
@brief Init Initialise this instance
*/ 
bool CImageLoader::Init(void)
{
	return false;
}

/**
@brief Load an image and return as unsigned char*
*/
unsigned char * CImageLoader::Load(const char* filename, int& width, int& height, int& nrChannels, const bool bInvert)
{
	// tell stb_image.h to flip loaded texture's on the y-axis.
	stbi_set_flip_vertically_on_load(bInvert);

	// load image, create texture and generate mipmaps
	unsigned char *data = stbi_load(FileSystem::getPath(filename).c_str(),
									&width, &height, &nrChannels, 0);

	// Return the data but you must remember to free up the memory of the file data read in
	return data;
}
