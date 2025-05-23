/*
*	Author: Eric Winebrenner
*/

#include "texture.h"
#include "external/glad.h"
#include "external/stb_image.h"

static int getTextureFormat(int numComponents) {
	switch (numComponents) {
	default:
		return GL_RGBA;
	case 3:
		return GL_RGB;
	case 2:
		return GL_RG;
	case 1:
		return GL_RED;
	}
}
namespace ew {
	unsigned int loadTexture(const char* filePath) {
		stbi_set_flip_vertically_on_load(true);
		return loadTexture(filePath, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
	}
	unsigned int loadTexture(const char* filePath, int wrapMode, int magFilter, int minFilter, bool mipmap) {
		int width, height, numComponents;
		unsigned char* data = stbi_load(filePath, &width, &height, &numComponents, 0);
		if (data == NULL) {
			printf("Failed to load image %s", filePath);
			stbi_image_free(data);
			return 0;
		}
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		int format = getTextureFormat(numComponents);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		//Black border by default
		float borderColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		if (mipmap) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);
		return texture;
	}
	unsigned int cubeMapTexture(vector<std::string>faces, int wrapMode, int magFilter, int minFilter) {
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

		int width, height, numComponents;
		for (int i = 0; i < faces.size(); i++)
		{
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &numComponents, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap Fail" <<std::endl;
				stbi_image_free(data);
			}
		}

		//Black border by default
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);

		return texture;
	}
}

