#pragma once
#include <string>

#include <glad/glad.h>

struct Texture
{
	GLuint TextureID = 0;
	uint32_t Width = 0;
	uint32_t Height = 0;
};

Texture CreateTexture(int width, int height);
Texture LoadTexture(const std::string filepath);

