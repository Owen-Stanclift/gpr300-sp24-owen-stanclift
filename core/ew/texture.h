/*
*	Author: Eric Winebrenner
*/

#pragma once
#include <vector>
#include <iostream>
#include <stdio.h>

using namespace std;

namespace ew {
	unsigned int loadTexture(const char* filePath);
	unsigned int loadTexture(const char* filePath, int wrapMode, int magFilter, int minFilter, bool mipmap);
	unsigned int cubeMapTexture(vector<std::string>faces , int wrapMode, int magFilter, int minFilter);
}
